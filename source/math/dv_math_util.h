#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class MathUtil
{
public:
	static bool floatEqual(float a, float b,
		float tolerance = std::numeric_limits<float>::epsilon()) {
		return std::abs(b - a) <= tolerance;
	}

	//Square root function.
	static float sqrt(float f) {
		return std::sqrt(f);
	}

	//min
	template<typename V>
	static V min2(const V& v1, const V& v2) {
		return (v1 < v2) ? v1 : v2;
	}

	template<typename V>
	static V min3(const V& v1, const V& v2, const V& v3) {
		return (v1 < v2) ? ((v1 < v3) ? v1 : v3) : ((v2 < v3) ? v2 : v3);
	}

	//max
	template<typename V>
	static V max2(const V& v1, const V& v2) {
		return (v1 > v2) ? v1 : v2;
	}

	template<typename V>
	static V max3(const V& v1, const V& v2, const V& v3) {
		return (v1 > v2) ? ((v1 > v3) ? v1 : v3) : ((v2 > v3) ? v2 : v3);
	}

	/* Simulate the shader function lerp which performers linear interpolation
	given 3 parameters v0, v1 and t the function returns the value of (1 - t)* v0 + t * v1.
	where v0 and v1 are matching vector or scalar types and t can be either a scalar or a
	vector of the same type as a and b.
	*/
	template <typename V, typename T> 
	static V lerp(const V& v0, const V& v1, const T& t) {
		return v0 * (1 - t) + v1 * t;
	}

	template <typename V, typename T>
	static V lerp3(const V& v0, const V& v1, const V& v2, const T& t) {
		return v0 * t[0] + v1 * t[1] + v2 * t[2];
	}

	// A random number in the range from [0,1]
	static float unitRandom(void);
	// A random number in the range from [low, high].
	static float rangeRandom(float low, float high) {
		return (high - low)*unitRandom() + low;
	}
	static int32_t rangeRandom(int32_t low, int32_t high) {
		return ((high - low) >= RAND_MAX) ? ((rand()<<16|rand()) % (high - low) + low) : (rand() % (high - low) + low);
	}
	//clamps the specified value within the range of min to max.
	template <typename T>
	static T saturate(T v, const T& min=(T)0, const T& max=(T)1) {
		return (v < min) ? min : ((v > max) ? max : v);
	}

	//cosine function
	static float cos(float a) {
		return std::cos(a);
	}
	//Sine function
	static float sin(float a) {
		return std::sin(a);
	}

	static const float POS_INFINITY;
	static const float NEG_INFINITY;
	static const float PI;
	static const float PI_X2;
	static const float PI_DIV2;
	static const float PI_DIV4;
	static const float fDeg2Rad;
	static const float fRad2Deg;
};

}
