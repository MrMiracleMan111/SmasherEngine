#pragma once
#include <list>
#include <functional>
#include <mutex>
#include "Smasher/Base.h"
#include "Smasher/JobManager/Job.h"
#include "Smasher/JobManager/JobPool.h"
#include "Smasher/JobManager/JobRunner.h"

namespace Smasher {
	class JobRunner;

	// Primary API for Jobs, feeds Jobs to the Job pool
	class SMASHER_API JobManager {
	friend class JobRunner;
	public:
		JobManager();
		JobManager(std::size_t numJobRunners);
		~JobManager();
		JobManager(const JobManager&) = delete;
		JobManager(JobManager &&other);
		JobManager& operator= (const JobManager&) = delete;
		JobManager& operator= (JobManager &&other);

		// Creates an asynchronous job, specify which jobs the new job is 
		// dependent on. 
		// There's no guarantee the returned reference
		// points to a valid Job. In the time between
		// constructing the refernce and copy assigning it,
		// the job could have been taken, completed, and removed
		Expected<std::reference_wrapper<Job>> AddAsyncJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies = {}, const char * jobName = "");

		// Creates a synchronous job, specify which jobs the new job is 
		// dependent on. This is Primarily for Render jobs.
		// Synchronous jobs CAN be dependant on asynchronous jobs
		// Synchronous jobs must NEVER be dependant on other synchronous jobs
		Expected<std::reference_wrapper<Job>> AddSyncJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies = {}, const char* jobName = "");


		// Launches runners, puts thread to sleep
		// until all jobs are completed then returns
		ErrorCode RunJobs();

		// Wait for all jobs to finish
		ErrorCode WaitForAsyncJobs();

		ErrorCode KillJobRunners();

		// Sets the "TickJobProducer" callback which will generate
		// jobs for every game tick.
		// 
		// "callback" will be run once at the start of each game tick.
		// "callback" should be used to add jobs to the Job Queue not
		// for component or game logic.
		ErrorCode SetTickJobProducer(std::function<void(void)> callback);

		void RunTickJobProducer() { m_TickJobProducer(); };
	protected:
		void FinishJob(Job& job); // Removes job from job pool and adds job descendants
								  // to available job list
	private:
		void InitializeRunners(std::size_t numRunners);
		std::function<void(void)> m_TickJobProducer = [](){};
		bool m_Valid = true;
		JobPool m_AsyncJobPool;
		JobPool m_SyncJobPool;
		std::forward_list<JobRunner> m_Runners;
		std::condition_variable m_RunnersStateCV;
		std::mutex m_RunnersStateMutex;
		std::mutex m_JobPoolsMutex;
	};

	static_assert(!std::is_copy_constructible_v<JobManager>);
	static_assert(!std::is_copy_assignable_v<JobManager>);
	static_assert(std::is_move_constructible_v<JobManager>);
	static_assert(std::is_move_assignable_v<JobManager>);
}