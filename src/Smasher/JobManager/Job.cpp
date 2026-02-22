#include <mutex>
#include <iostream>
#include "Smasher/JobManager/Job.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	int Job::s_JobCount = 1;

	Job::~Job() {
	}

	Job::Job(std::reference_wrapper<JobPool> pool, std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies) :
		m_Callback(callback),
		m_JobId(Job::s_JobCount),
		m_JobPoolRef(pool),
		m_Valid(true)
	{
		Job::s_JobCount++;
		std::scoped_lock lock1(m_StateMutex);
		m_Status = JobStatus::WAITING;
		for (auto parent : dependencies) {
			std::scoped_lock lock(parent.get().GetMutex());
			// Throws exception
			parent.get().AddDependant(*this);
		}
		m_NumParents += (unsigned int)dependencies.size();
	}

	Job::Job(Job&& other) noexcept :
		m_NumParents(std::move(other.m_NumParents)),
		m_Dependants(std::move(other.m_Dependants)),
		m_AllJobsListItr(std::move(other.m_AllJobsListItr)),
		m_Callback(std::move(other.m_Callback)),
		m_Status(std::move(other.m_Status)),
		m_JobPoolRef(std::move(other.m_JobPoolRef)),
		m_JobId(other.m_JobId),
		m_Valid(other.m_Valid)
	{
		// Job should not be locked when trying to move
		// The code below doesn't work since try_lock() spuriously fails
		// also, the lock needs to be released
		// assert(other.m_StateMutex.try_lock());
		// other.m_StateMutx.unlock();
		other.m_Valid = false;
	}

	Job& Job::operator= (Job&& other) noexcept {
		// Other job should not be locked
		// assert(other.m_StateMutex.try_lock());
		// other.m_StateMutex.unlock();

		if (this != &other) {
			m_Valid = other.m_Valid;
			m_NumParents = std::move(other.m_NumParents);
			m_Dependants = std::move(other.m_Dependants);
			m_AllJobsListItr = std::move(other.m_AllJobsListItr);
			m_Callback = std::move(other.m_Callback);
			m_Status = std::move(other.m_Status);
			m_JobPoolRef = std::move(other.m_JobPoolRef);
			m_JobId = other.m_JobId;
			other.m_Valid = false;
		}
		return *this;
	}

	void Job::AddDependant(Job& other) {
		m_Dependants.emplace_back(other);
	}

	ErrorCode Job::Run() {
		m_Status = JobStatus::RUNNING;
		ErrorCode code = m_Callback();
		m_Status = JobStatus::DONE;
		return code;
	}

	ErrorCode Job::RemoveParent() {
		if (m_NumParents <= 0) {
			return ERROR_JobHasNoParents;
		}
		m_NumParents--;
		return ERROR_NoError;
	}
}