#include <mutex>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "Smasher/JobManager/JobManager.h"
#include "Smasher/JobManager/Job.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	JobManager::JobManager() :
		m_AsyncJobPool(),
		m_SyncJobPool(),
		m_Valid(true),
		m_Runners()
	{
		std::size_t count = std::max(std::thread::hardware_concurrency() - 2u, 2u);
		InitializeRunners(count);
	}

	JobManager::JobManager(std::size_t numJobRunners) :
		m_AsyncJobPool(),
		m_SyncJobPool(),
		m_Valid(true),
		m_Runners()
	{
		InitializeRunners(numJobRunners);
	}

	JobManager::~JobManager() {
		if (m_Valid) {
			KillJobRunners();
			m_Runners.clear();
		}
	}

	JobManager::JobManager(JobManager &&other) :
		m_Runners(std::move(other.m_Runners)),
		m_AsyncJobPool(std::move(other.m_AsyncJobPool))
	{
		// Runners would have invalid references to m_JobPool
		// after m_JobPool is moved
		// So as a precondition we need m_Runners to be empty
		assert(m_Runners.empty());
		other.m_Valid = false;
	}

	JobManager& JobManager::operator= (JobManager &&other) {
		// Runners would have invalid references to m_JobPool
		// after m_JobPool is moved
		// So as a precondition we need m_Runners to be empty
		assert(m_Runners.empty());

		if (this != &other) {
			other.m_Valid = false;
			m_Runners = std::move(other.m_Runners);
			m_AsyncJobPool = std::move(other.m_AsyncJobPool);
		}
		return *this;
	}

	void JobManager::InitializeRunners(std::size_t numRunners) {
		for (std::size_t i = 0; i < numRunners; ++i) {
			m_Runners.emplace_front(m_AsyncJobPool, m_RunnersStateCV, *this);
		}
		m_Runners.emplace_front(m_SyncJobPool, m_RunnersStateCV, *this, true);
	}

	Expected<std::reference_wrapper<Job>> JobManager::AddAsyncJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies) {
		//std::scoped_lock lock(m_AsyncJobPool.GetMutex());
		std::scoped_lock lock(m_AsyncJobPool.GetCVMutex());
		return m_AsyncJobPool.AddJob(callback, dependencies);
	}

	Expected<std::reference_wrapper<Job>> JobManager::AddSyncJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies) {
		//std::scoped_lock lock(m_SyncJobPool.GetMutex());
		std::scoped_lock lock(m_SyncJobPool.GetCVMutex());
		return m_SyncJobPool.AddJob(callback, dependencies);
	}

	void JobManager::FinishJob(Job& job) {
		std::lock(m_JobPoolsMutex, job.GetMutex(), m_SyncJobPool.GetCVMutex(), m_AsyncJobPool.GetCVMutex());
		std::unique_lock<std::mutex> jobPoolsMutex(m_JobPoolsMutex, std::adopt_lock);
		std::unique_lock<std::mutex> jobMutex(job.GetMutex(), std::adopt_lock);
		std::unique_lock<std::mutex> syncJobPoolMutex(m_SyncJobPool.GetCVMutex(), std::adopt_lock);
		std::unique_lock<std::mutex> asyncJobPoolMutex(m_AsyncJobPool.GetCVMutex(), std::adopt_lock);

		for (Job& dependant : job.GetDependants()) {
			ErrorCode ret = dependant.RemoveParent();
			assert((ret == ERROR_NoError) && "Remove Parent failed");
			if (dependant.GetParentCount() == 0) {
				dependant.GetJobPool().AddAvailableJob(dependant);
				dependant.GetJobPool().GetCV().notify_all();
			}
		}
		// Job is about to be deleted and go out of scope
		// so we need to unlock its mutex
		jobMutex.unlock();

		// TODO: Error could occur here
		// if another thread tries to access the job
		job.GetJobPool().RemoveFromAllJobs(job);
	}

	ErrorCode JobManager::RunJobs() {
		for (auto& runner : m_Runners) {
			runner.Activate();
		}

		// Wakeup all job runners
		m_AsyncJobPool.GetCV().notify_all();

		for (auto& runner : m_Runners) {
			if (runner.IsSynchronous()) {
				runner.Run();
			}
		}
		return ERROR_NoError;
	}

	ErrorCode JobManager::WaitForAsyncJobs() {
		std::unique_lock<std::mutex> lock(m_RunnersStateMutex);
		m_RunnersStateCV.wait(lock, [this] {
			if (!m_AsyncJobPool.IsPoolEmpty()) {
				return false;
			}

			for (auto& runner : m_Runners) {
				if (runner.GetState() == JobRunnerSTATE::RUNNING) {
					return false;
				}
			}
			return true;
		});
		return ERROR_NoError;
	}

	ErrorCode JobManager::KillJobRunners() {
		for (auto &job : m_Runners) {
			job.Kill();
		}

		// Wakeup all job runners
		m_AsyncJobPool.GetCV().notify_all();
		m_SyncJobPool.GetCV().notify_all();

		for (auto &job : m_Runners) {
			if (!job.IsSynchronous()) {
				job.GetThread().join();
			}
		}

		return ERROR_NoError;
	}

	ErrorCode JobManager::SetTickJobProducer(std::function<void(void)> callback) {
		m_TickJobProducer = callback;
		return ERROR_NoError;
	}
}