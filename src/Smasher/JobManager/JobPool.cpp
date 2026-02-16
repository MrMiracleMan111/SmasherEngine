#include <mutex>
#include "Smasher/JobManager/JobPool.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Exceptions.h"

namespace Smasher {
	JobPool::JobPool() :
		m_Valid(true)
	{

	}

	JobPool::~JobPool() {
		if (m_Valid) {
			m_AvailableJobs.clear();
			m_AllJobs.clear();
		}
	}

	JobPool::JobPool(JobPool&& other) :
		m_AllJobs(std::move(other.m_AllJobs)),
		m_AvailableJobs(std::move(other.m_AvailableJobs))
	{
		other.m_Valid = false;
	}

	JobPool& JobPool::operator= (JobPool&& other) {
		if (this != &other) {
			other.m_Valid = false;
			m_AllJobs = std::move(other.m_AllJobs);
			m_AvailableJobs = std::move(other.m_AvailableJobs);
		}
		return *this;
	}


	Expected<std::reference_wrapper<Job>> JobPool::AddJob(std::function<ErrorCode(void)> callback, std::initializer_list<std::reference_wrapper<Job>> dependencies) {
		Job& job = *m_AllJobs.emplace(m_AllJobs.end(), std::ref(*this), callback, dependencies);
		job.SetItr(--(m_AllJobs.end()));
		if (dependencies.size() == 0) {
			m_AvailableJobs.push_back(job);
		}
		return std::ref(job);
	}

	Expected<Job> JobPool::TakeAvailableJob() {
		if (!IsJobAvailable()) {
			return Expected<Job>::Error(ERROR_NoJobsAvailable);
		}

		Job& jobRef = m_AvailableJobs.front();
		Job job = std::move(*jobRef.GetItr()); // Remove from m_AllJobs
		m_AllJobs.erase(job.GetItr());
		m_AvailableJobs.pop_front();
		return std::move(job);
	}
}