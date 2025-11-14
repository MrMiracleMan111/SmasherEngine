namespace Smasher {
	template<typename T>
	Interpolated<T>::Interpolated() :
		m_StartValue(),
		m_EndValue(),
		m_StartTime(std::chrono::system_clock::now()),
		m_Duration(Smasher::Millisecond{ 0 }),
		m_Curve(InterpolationType::INSTANT) 
	{

	};

	template<typename T>
	Interpolated<T>::Interpolated(const T& startValue, const T& endValue) :
		m_StartValue(startValue),
		m_EndValue(endValue),
		m_StartTime(std::chrono::system_clock::now()),
		m_Duration(Smasher::Millisecond{ 1000 }),
		m_Curve(InterpolationType::EASE_IN)
	{
		
	}

	template<typename T>
	Interpolated<T>::Interpolated(const T& value) :
		m_StartValue(value),
		m_EndValue(value),
		m_StartTime(std::chrono::system_clock::now()),
		m_Duration(Smasher::Millisecond{ 1000 }),
		m_Curve(InterpolationType::EASE_IN)
	{

	}

	template<typename T>
	Interpolated<T>::operator T() { return calculateValue(); }

	template<typename T>
	T Interpolated<T>::Get() { return calculateValue(); }

	template<typename T>
	void Interpolated<T>::Set(const T& value) {
		m_StartValue = value;
		m_StartTime = std::chrono::system_clock::now();
		m_EndValue = value;
	}

	template<typename T>
	Interpolated<T>& Interpolated<T>::operator=(const T& endValue) {
		m_StartValue = calculateValue();
		m_StartTime = std::chrono::system_clock::now();
		m_EndValue = endValue;
		return *this;
	}

	template<typename T>
	T Interpolated<T>::calculateValue() {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		Smasher::Millisecond elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_StartTime);
		float clamped = (float)std::clamp<long long>(elapsed.count(), 0, m_Duration.count());
		float duration = (float)m_Duration.count();

		if (duration == 0.0f) {
			return m_EndValue;
		}

		float percent = InterpolationUtils::getInterpolationValue(m_Curve, clamped / duration);
		return percent * (m_EndValue - m_StartValue) + m_StartValue;
	}
}