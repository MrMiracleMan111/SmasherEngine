#include "Smasher/Base.h"
#include "Smasher/Interpolation.h"

namespace Smasher {
	namespace InterpolationUtils {
		float getInterpolationValue(InterpolationType curve, float x) {
			switch (curve) {
				case InterpolationType::LINEAR:
					return linear(x);
				case InterpolationType::EASE_IN:
					return easeIn(x);
				case InterpolationType::EASE_OUT:
					return easeOut(x);
				case InterpolationType::EASE_IN_OUT:
					return easeInOut(x);
				case InterpolationType::STEP:
					return step(x);
				case InterpolationType::INSTANT:
					return 1.f;
				default:
					assert(false); // "This case should never be reached
					return -1.f;
			}
			return 1.f;
		}

		float linear(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return x;
		}

		/**
		*
		* Using https://easings.net for easing functions
		*
		*/

		float easeIn(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return clamped * clamped * clamped;;
		}

		float easeOut(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return 1 - std::pow(1 - clamped, 3);
		}

		float easeInOut(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return clamped < 0.5f ? 4.f * clamped * clamped * clamped : 1.f - std::pow(-2.f * clamped + 2.f, 3.f) / 2.f;
		}

		float step(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return (clamped < 0.5f) ? 0.f : 1.f;
		}
	}
}