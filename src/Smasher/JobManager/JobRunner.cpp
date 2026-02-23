#include <iostream>
#include "Smasher/JobManager/JobRunner.h"
#include "Smasher/JobManager/JobManager.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	JobRunner::JobRunner(JobPool& jobPool, std::condition_variable &runnerStateCV, JobManager &manager, bool synchronous) :
		m_JobPool(jobPool),
		m_RunnerStateCVRef(runnerStateCV),
		m_Synchronous(synchronous),
		m_JobManagerRef(manager),
		m_Thread()
	{
		if (!synchronous) {
			m_Thread = std::move(std::thread{ &JobRunner::Run, this });
		}
	}

	JobRunner::~JobRunner() {

	}

	JobRunner::JobRunner(JobRunner&& other) :
		m_Dead(std::move(other.m_Dead)),
		m_JobPool(other.m_JobPool),
		m_Thread(std::move(other.m_Thread)),
		m_Activated(other.m_Activated),
		m_RunnerStateCVRef(other.m_RunnerStateCVRef),
		m_JobManagerRef(std::move(other.m_JobManagerRef))
	{
		Smasher::JobRunnerSTATE state = other.m_StateAtomic;
		m_StateAtomic = state;
	}

	JobRunner& JobRunner::operator= (JobRunner&& other) {
		if (this != &other) {
			m_Dead = std::move(other.m_Dead);
			m_Thread = std::move(other.m_Thread);
			m_RunnerStateCVRef = std::move(other.m_RunnerStateCVRef);
			m_Activated = other.m_Activated;
			m_JobManagerRef = std::move(other.m_JobManagerRef);
			Smasher::JobRunnerSTATE state = other.m_StateAtomic;
			m_StateAtomic = state;
		}
		return *this;
	}

	void JobRunner::Activate() {
		m_Activated = true;
	}


	void JobRunner::Run() {
		try {
			_Run();
		}
		catch (const std::exception& e) {
			std::cerr << "JobRunner Exception thrown: " << e.what() << "\n";
		}
		catch (...) {
			std::cerr << "Something went wrong in JobRunner \n";
		}
	}

	void JobRunner::SignalWaiting() {
		m_RunnerStateCVRef.get().notify_all();
	}


	void JobRunner::_Run() {
		// Wait for initial signal to start running
		if (!m_Synchronous) {
			m_StateAtomic = JobRunnerSTATE::WAITING;
			std::unique_lock<std::mutex> cvLock(m_JobPool.GetCVMutex());
			m_JobPool.GetCV().wait(cvLock, [&]() {
				return m_Activated || m_Dead;
			});
			cvLock.unlock();
		}

		// Could be spuriously woken up
		while (!m_Dead) {
			std::unique_lock<std::mutex> lock1(m_JobPool.GetCVMutex());

			// Exit loop for synchronous job runners
			if (m_Synchronous && m_JobPool.IsPoolEmpty()) {
				break;
			}
			// Sleep until another job is available
			if (!m_JobPool.IsJobAvailable()) {
				m_StateAtomic = JobRunnerSTATE::WAITING;
				SignalWaiting();
				m_JobPool.GetCV().wait(lock1);
				continue; // lock1 goes out of scope and unlocks
			}

			m_StateAtomic = JobRunnerSTATE::RUNNING;
			Expected<std::reference_wrapper<Job>> ret = m_JobPool.TakeAvailableJob();
			lock1.unlock();

			if (!ret) {
				// Handle error
				break;
			}
			Job& job = ret.Get();

			ErrorCode code = job.Run();

			// Add all job dependants to the available job queue
			// if their parent count is now 0
			// Job dependants may belong to different pools
			// TODO: This will cause deadlock
			// Thread A locks (JobPool or Job) A
			// Thread B locks (JobPool or Job) B
			// Thread A tries to lock (JobPool or Job) B
			// Thread A waits on Thread B
			// Thread B tries to lock (JobPool or Job) A
			// Thread B waits on Thread A
			// DEADLOCK
			m_JobManagerRef.get().FinishJob(job);
			m_StateAtomic = JobRunnerSTATE::WAITING;
			SignalWaiting();
		}
	}

	void JobRunner::Kill() {
		m_Dead = true;
	}
}