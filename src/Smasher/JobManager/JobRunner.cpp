#include <iostream>
#include "Smasher/JobManager/JobRunner.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	JobRunner::JobRunner(JobPool& jobPool, std::condition_variable &runnerStateCV, bool synchronous) :
		m_JobPool(jobPool),
		m_RunnerStateCVRef(runnerStateCV),
		m_Synchronous(synchronous),
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
		m_RunnerStateCVRef(other.m_RunnerStateCVRef)
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
		while (!m_Dead && !m_JobPool.IsPoolEmpty()) {
			std::unique_lock<std::mutex> lock1(m_JobPool.GetMutex());
			// Sleep until another job is available
			if (!m_JobPool.IsJobAvailable()) {
				lock1.unlock();
				m_StateAtomic = JobRunnerSTATE::WAITING;
				SignalWaiting();
				std::unique_lock<std::mutex> cvLock(m_JobPool.GetCVMutex());
				m_JobPool.GetCV().wait(cvLock);
				cvLock.unlock();
				continue;
			}

			m_StateAtomic = JobRunnerSTATE::RUNNING;
			Expected<Job> ret = m_JobPool.TakeAvailableJob();
			lock1.unlock();

			if (!ret) {
				// Handle error
				break;
			}

			ErrorCode code = ret.Get().Run();

			// Add all job dependants to the available job queue
			// if their parent count is now 0
			// Job dependants may belong to different pools
			std::scoped_lock lock2(m_JobPool.GetMutex());
			for (Job &dependant : ret.Get().GetDependants()) {
				ErrorCode ret;

				// Extra logic if dependant is in different job pool
				if (&m_JobPool != &dependant.GetJobPool()) {
					dependant.GetJobPool().GetMutex().lock();
				}

				std::scoped_lock dependantLock(dependant.GetMutex());
				// Job was added to available job list by another JobRunner
				if (dependant.GetParentCount() == 0) {
					if (&m_JobPool != &dependant.GetJobPool()) {
						dependant.GetJobPool().GetMutex().unlock();
					}
					
					// same as "continue"
					goto unlock_dependant_job_pool;
				}

				ret = dependant.RemoveParent();
				assert((ret == ERROR_NoError) && "Remove Parent failed");
				if (dependant.GetParentCount() == 0) {
					dependant.GetJobPool().AddAvailableJob(dependant);
					dependant.GetJobPool().GetCV().notify_one();
				}

			unlock_dependant_job_pool:
				if (&m_JobPool != &dependant.GetJobPool()) {
					dependant.GetJobPool().GetMutex().unlock();
				}
			}
			m_StateAtomic = JobRunnerSTATE::WAITING;
			SignalWaiting();
		}
		m_Dead = true;
	}

	void JobRunner::Kill() {
		m_Dead = true;
	}
}