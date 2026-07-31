#pragma once
#include <cmath>
#define M_PI 3.14159265358979323846

namespace ease
{
	inline float linear(float t)
	{
		return t;
	}

	inline float quadIn(float t)
	{
		return t * t;
	}

	inline float quadOut(float t)
	{
		return t * (2.0f - t);
	}

	inline float quadInOut(float t)
	{
		if (t < 0.5f)
			return 2.0f * t * t;

		return 1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f;
	}
}

namespace feel
{
	enum class EasingType{ In, Out, InOut };

	struct ICurve
	{
		ICurve() = default;
		virtual ~ICurve() = default;
		virtual float sample(float t) = 0;
	};

	struct LinearCurve : public ICurve
	{
		LinearCurve() = default;
		float sample(float t) override
		{
			ease::linear(t);
		}
	};

	struct QuadCurve : public ICurve
	{
		EasingType type = EasingType::In;

		QuadCurve() = default;
		QuadCurve(EasingType type)
		{
			this->type = type;
		}

		float sample(float t) override
		{
			switch (type)
			{
			case EasingType::In:	return ease::quadIn(t);
			case EasingType::Out:	return ease::quadOut(t);
			case EasingType::InOut: return ease::quadInOut(t);
			}
		}
	};

	struct HillCurve : public ICurve
	{
		HillCurve() = default;
		float sample(float t) override
		{
			return std::sin(M_PI * t);
		}
	};
}