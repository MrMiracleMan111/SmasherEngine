#pragma once
#include <mutex>
#include "Smasher/Exceptions.h"
#include "Smasher/JobManager/Job.h"
#include "Smasher/JobManager/JobPool.h"

namespace Smasher {
	class JobManager;
	enum class JobRunnerSTATE {
		WAITING,
		RUNNING,
	};

	// Takes Jobs from the job pool
	class JobRunner {
	public:
		JobRunner() = delete;
		JobRunner(JobPool &jobPool, std::condition_variable &runnerStateCV);
		~JobRunner();
		JobRunner(const JobRunner&) = delete;
		JobRunner(JobRunner&& other);
		JobRunner& operator= (const JobRunner&) = delete;
		JobRunner& operator= (JobRunner&& other);

		// While loop, waits for available jobs
		void Run();

		void Activate();

		void Kill();

		std::thread& GetThread() { return m_Thread; }
		const JobRunnerSTATE GetState() const { return m_StateAtomic; }
	private:
		void _Run();
		// Notify main thread, that JobRunner thread is waiting
		void SignalWaiting();
		bool m_Activated = false;
		bool m_Dead = false;
		JobPool& m_JobPool;
		std::thread m_Thread;
		std::reference_wrapper<std::condition_variable> m_RunnerStateCVRef;
		std::atomic<JobRunnerSTATE> m_StateAtomic { JobRunnerSTATE::WAITING };
	};

	static_assert(!std::is_copy_constructible_v<JobRunner>);
	static_assert(!std::is_copy_assignable_v<JobRunner>);
	static_assert(std::is_move_constructible_v<JobRunner>);
	static_assert(std::is_move_assignable_v<JobRunner>);
}