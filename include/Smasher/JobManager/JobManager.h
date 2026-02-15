#pragma once
#include <list>
#include <functional>
#include <mutex>
#include "Smasher/Base.h"
#include "Smasher/JobManager/Job.h"
#include "Smasher/JobManager/JobPool.h"
#include "Smasher/JobManager/JobRunner.h"

namespace Smasher {
	// Primary API for Jobs, feeds Jobs to the Job pool
	class SMASHER_API JobManager {
	public:
		JobManager();
		JobManager(unsigned int numJobRunners);
		~JobManager();
		JobManager(const JobManager&) = delete;
		JobManager(JobManager &&other);
		JobManager& operator= (const JobManager&) = delete;
		JobManager& operator= (JobManager &&other);

		// Creates a job, specify which jobs the new job is 
		// dependent on. 
		// There's no guarantee the returned reference
		// points to a valid Job. In the time between
		// constructing the refernce and copy assigning it,
		// the job could have been taken, completed, and removed
		Expected<std::reference_wrapper<Job>> CreateJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies = {});
		
		// Launches runners, puts thread to sleep
		// until all jobs are completed then returns
		ErrorCode RunJobs();

		// Wait for all jobs to finish
		ErrorCode WaitForJobs();

		ErrorCode KillJobRunners();

	private:
		void InitializeRunners(unsigned int numRunners);
		bool m_Valid = true;
		JobPool m_JobPool;
		std::forward_list<JobRunner> m_Runners;
		std::condition_variable m_RunnersStateCV;
		std::mutex m_RunnersStateMutex;
	};

	static_assert(!std::is_copy_constructible_v<JobManager>);
	static_assert(!std::is_copy_assignable_v<JobManager>);
	static_assert(std::is_move_constructible_v<JobManager>);
	static_assert(std::is_move_assignable_v<JobManager>);
}