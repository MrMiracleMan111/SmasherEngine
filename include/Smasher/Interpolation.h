#pragma once
#include "Base.h"

namespace Smasher {

	enum class InterpolationType {
		LINEAR,
		EASE_IN,
		EASE_OUT,
		EASE_IN_OUT,
		STEP,
		INSTANT
	};

	namespace InterpolationUtils {
		float SMASHER_API getInterpolationValue(InterpolationType curve, float x);
		float SMASHER_API linear(float x);
		float SMASHER_API easeIn(float x);
		float SMASHER_API easeOut(float x);
		float SMASHER_API easeInOut(float x);
		float SMASHER_API step(float x);
		float SMASHER_API cubicBezier(float t, float p0, float p1, float p2, float p3);
	};

	template <typename T>
	class Interpolated {
	public:
		Interpolated();
		Interpolated(const T& startValue, const T& endValue);

		const Smasher::Millisecond& GetDuration() const { return m_Duration; };
		void SetDuration(Smasher::Millisecond duration) { m_Duration = duration; };

		operator T();
		T Get();

		void Set(const T& endValue);
		void SetCurve(InterpolationType type) { m_Curve = type; }
		Interpolated& operator=(const T&);
	private:
		T calculateValue();

		T m_StartValue;
		T m_EndValue;
		InterpolationType m_Curve;
		std::chrono::time_point<std::chrono::system_clock> m_StartTime;
		Smasher::Millisecond m_Duration = 1000;
	};
}

#include "Smasher/Interpolation.inl"