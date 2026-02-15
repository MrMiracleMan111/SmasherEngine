#include <mutex>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "Smasher/JobManager/JobManager.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	JobManager::JobManager() :
		m_JobPool(),
		m_Valid(true),
		m_Runners()
	{
		std::size_t count = std::max(std::thread::hardware_concurrency() - 2u, 2u);
		InitializeRunners(count);
	}

	JobManager::JobManager(unsigned int numJobRunners) :
		m_JobPool(),
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
		m_JobPool(std::move(other.m_JobPool))
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
			m_JobPool = std::move(other.m_JobPool);
		}
		return *this;
	}

	void JobManager::InitializeRunners(unsigned int numRunners) {
		std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
		for (std::size_t i = 0; i < numRunners; ++i) {
			m_Runners.emplace_front(m_JobPool, m_RunnersStateCV);
		}
		std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
		std::cout << "JobManager initialization time: " << diff.count() << "ms\n";
	}

	Expected<std::reference_wrapper<Job>> JobManager::CreateJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies) {
		std::scoped_lock lock(m_JobPool.GetMutex());
		return m_JobPool.AddJob(callback, dependencies);
	}

	ErrorCode JobManager::RunJobs() {
		for (auto& job : m_Runners) {
			job.Activate();
		}

		// Wakeup all job runners
		m_JobPool.GetCV().notify_all();
		return ERROR_NoError;
	}

	ErrorCode JobManager::WaitForJobs() {
		std::unique_lock<std::mutex> lock(m_RunnersStateMutex);
		m_RunnersStateCV.wait(lock, [this] {
			if (m_JobPool.IsJobAvailable()) {
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
		std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();

		for (auto &job : m_Runners) {
			job.Kill();
		}

		// Wakeup all job runners
		m_JobPool.GetCV().notify_all();

		for (auto &job : m_Runners) {
			job.GetThread().join();
		}

		std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
		std::cout << "JobManager KillJobRunners time: " << diff.count() << "ms\n";
		return ERROR_NoError;
	}
}