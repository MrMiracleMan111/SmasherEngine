#pragma once
#include <vector>
#include <list>
#include <functional>
#include <mutex>
#include "Smasher/Base.h"
#include "Smasher/Exceptions.h"

namespace Smasher {
	class JobPool;
	enum class JobStatus {
		WAITING,
		RUNNING,
		DONE
	};

	class SMASHER_API Job {
		friend class JobManager;
		friend class JobPool;
		friend class JobRunner;

	public:
		Job() = delete;
		Job(std::reference_wrapper<JobPool> pool,
			std::function<ErrorCode(void)> callback,
			std::initializer_list<std::reference_wrapper<Job>> dependencies); // Called by JobManager
		~Job();
		Job(const Job&) = delete;
		Job(Job&& other) noexcept;
		Job& operator= (const Job&) = delete;
		Job& operator= (Job&& other) noexcept;

		const JobStatus GetStatus() const { return m_Status; }
		const int GetJobId() const { return m_JobId; }
		JobPool& GetJobPool() { return m_JobPoolRef.get(); }

		static void ResetJobCount() { Job::s_JobCount = 1; };
	protected:
		ErrorCode Run();

		std::vector<std::reference_wrapper<Job>>& GetDependants() { return m_Dependants;  };
		
		const unsigned int GetParentCount() { return m_NumParents; };

		// Decrement parent counter
		// throws error if the job has no parents
		ErrorCode RemoveParent();

		void SetItr(const std::list<Job>::iterator& itr) { m_AllJobsListItr = itr; };

		std::list<Job>::iterator GetItr() { return m_AllJobsListItr; };
		std::mutex& GetMutex() { return m_StateMutex; };
	private:
		// Forces this job to wait on other job before running
		// Adds this job to m_Dependants of "other"
		void AddDependant(Job& other);

		std::reference_wrapper<JobPool> m_JobPoolRef; // Job pool that owns this job
		static int s_JobCount;
		int m_JobId;
		std::mutex m_StateMutex;
		unsigned int m_NumParents = 0; // Number of jobs this one waits on
		JobStatus m_Status;
		bool m_Valid = true; // For tracking moves

		std::function<ErrorCode(void)> m_Callback;
		std::vector<std::reference_wrapper<Job>> m_Dependants; // Jobs waiting on this one

		std::list<Job>::iterator m_AllJobsListItr; // Iterator for this job in "AllJobs list"
	};

	static_assert(!std::is_copy_constructible_v<Smasher::Job>);
	static_assert(!std::is_copy_assignable_v<Smasher::Job>);
	static_assert(std::is_move_constructible_v<Smasher::Job>);
	static_assert(std::is_move_assignable_v<Smasher::Job>);
}