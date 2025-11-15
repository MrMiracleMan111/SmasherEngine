#include <numbers>
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
				case InterpolationType::EASE_IN_ELASTIC:
					return easeInElastic(x);
				case InterpolationType::EASE_OUT_ELASTIC:
					return easeOutElastic(x);
				case InterpolationType::EASE_IN_OUT_ELASTIC:
					return easeInOutElastic(x);
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

		float SMASHER_API easeInElastic(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			const float c4 = (2.f * (float)std::numbers::pi) / 3.f;

			if (clamped == 0.f) {
				return 0.f;
			}
			else if (clamped == 1.f) {
				return 1.f;
			}
			return 1.f - std::pow(2.f, 10.f * clamped - 10.f) * std::sin((clamped * 10.f - 10.75f) * c4);
		}
		
		float SMASHER_API easeOutElastic(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			const float c4 = (2.f * (float)std::numbers::pi) / 3.f;

			if (clamped == 0.f) {
				return 0.f;
			}
			else if (clamped == 1.f) {
				return 1.f;
			}
			return std::pow(2.f, -10.f * clamped) * std::sin((clamped * 10.f - 0.75f) * c4) + 1.f;
		}
		
		float SMASHER_API easeInOutElastic(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			const float c5 = (2.f * (float)std::numbers::pi) / 4.5f;

			if (clamped == 0.f) {
				return 0.f;
			}
			else if (clamped == 1.f) {
				return 1.f;
			}
			else if (clamped < 0.5f) {
				return -(std::pow(2.f, 20.f * clamped - 10.f) * std::sin((20.f * clamped - 11.125f) * c5)) / 2.f;
			}
			return (std::pow(2.f, -20.f * clamped + 10.f) * std::sin((20.f * clamped - 11.125f) * c5)) / 2.f + 1.f;
		}

		
		float step(float x) {
			float clamped = std::clamp(x, 0.f, 1.f);
			return (clamped < 0.5f) ? 0.f : 1.f;
		}
	}
}