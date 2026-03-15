#pragma once
#include <list>
#include <vector>
#include <mutex>
#include "Smasher/JobManager/Job.h"

namespace Smasher {
	class JobPool {
		friend class JobManager;
		friend class JobRunner;
	
	public:
		~JobPool();
		JobPool(const JobPool&) = delete;
		JobPool(JobPool&& other);
		JobPool& operator= (const JobPool&) = delete;
		JobPool& operator= (JobPool&& other);

	protected:
		JobPool();

		// Add job and return reference to that job
		// @precondition m_PoolStateMutex must be locked by calling function
		Expected<std::reference_wrapper<Job>> AddJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies);

		// Add job to Available Jobs list
		void AddAvailableJob(Job& job) { m_AvailableJobs.push_back(job); };

		const bool IsJobAvailable() const { return !m_AvailableJobs.empty(); };

		const bool IsPoolEmpty() const { return m_AllJobs.empty(); };

		void RemoveFromAllJobs(Job &job);

		/**
		 * @brief Removes job from m_AvailableJobs
		 * 
		 * @pre m_PoolStateMutex must be locked by caller.
		 */
		Expected<std::reference_wrapper<Job>> TakeAvailableJob();

		std::mutex& GetCVMutex() { return m_PoolCVMutex; }

		std::condition_variable& GetCV() { return m_PoolCV; }

	private:
		bool m_Valid = true;
		std::mutex m_PoolCVMutex; 
		std::condition_variable m_PoolCV;
		std::list<Job> m_AllJobs;
		std::list<std::reference_wrapper<Job>> m_AvailableJobs;
	};

	static_assert(!std::is_copy_constructible_v<JobPool>);
	static_assert(!std::is_copy_assignable_v<JobPool>);
	static_assert(std::is_move_constructible_v<JobPool>);
	static_assert(std::is_move_assignable_v<JobPool>);
}