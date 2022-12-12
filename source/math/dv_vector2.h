#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

template<typename T>
class TVector2
{
public:
	T x, y;

public:
	// Construct
	TVector2() : x(0), y(0) { }
	TVector2(T _x, T _y) : x(_x), y(_y){ }
	inline explicit TVector2(const T s) : x(s), y(s) { }
	inline explicit TVector2(const T f[2]) : x(f[0]), y(f[1]) { }
	inline explicit TVector2(T* const f) : x(f[0]), y(f[1]) { }
	inline explicit TVector2(const int f[2]) : x((T)f[0]), y((T)f[1]) { }

public:
	inline T operator [] (const size_t i) const {
		assert(i < 2);
		return *(&x + i);
	}

	inline T& operator [] (const size_t i) {
		assert(i < 2);
		return *(&x + i);
	}

	inline T* ptr(void) {
		return &x;
	}

	inline const T* ptr(void) const {
		return &x;
	}
public:
	// Operations
	inline TVector2& operator = (const TVector2& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		return *this;
	}

	inline TVector2& operator = (const T fScalar) {
		x = fScalar;
		y = fScalar;
		return *this;
	}

	inline bool operator == (const TVector2& rkVector) const {
		return (x == rkVector.x && y == rkVector.y);
	}

	inline bool operator != (const TVector2& rkVector) const {
		return (x != rkVector.x || y != rkVector.y);
	}

public:
	// Arithmetic operations
	inline TVector2 operator + (const TVector2& rkVector) const {
		return TVector2( x + rkVector.x, y + rkVector.y);
	}

	inline TVector2 operator - (const TVector2& rkVector) const {
		return TVector2( x - rkVector.x, y - rkVector.y);
	}

	inline TVector2 operator * (const T fScalar) const {
		return TVector2( x * fScalar, y * fScalar);
	}

	inline TVector2 operator * (const TVector2& rhs) const {
		return TVector2(x * rhs.x, y * rhs.y);
	}

	inline TVector2 operator / (const T fScalar) const {
		assert(fScalar != 0.0);
		return TVector2(x / fScalar, y / fScalar);
	}

	inline TVector2 operator / (const TVector2& rhs) const {
		return TVector2( x / rhs.x, y / rhs.y);
	}

	inline const TVector2& operator + () const {
		return *this;
	}

	inline TVector2 operator - () const {
		return TVector2(-x, -y);
	}

public:
	// overloaded operators to help TVector2
	inline friend TVector2 operator * (const T fScalar, const TVector2& rkVector) {
		return TVector2(fScalar * rkVector.x, fScalar * rkVector.y);
	}

	inline friend TVector2 operator / (const T fScalar, const TVector2& rkVector) {
		return TVector2( fScalar / rkVector.x, fScalar / rkVector.y);
	}

	inline friend TVector2 operator + (const TVector2& lhs, const T rhs) {
		return TVector2( lhs.x + rhs, lhs.y + rhs);
	}

	inline friend TVector2 operator + (const T lhs, const TVector2& rhs) {
		return TVector2( lhs + rhs.x, lhs + rhs.y);
	}

	inline friend TVector2 operator - (const TVector2& lhs, const T rhs) {
		return TVector2( lhs.x - rhs, lhs.y - rhs);
	}

	inline friend TVector2 operator - (const T lhs, const TVector2& rhs) {
		return TVector2( lhs - rhs.x, lhs - rhs.y);
	}

public:
	// arithmetic updates
	inline TVector2& operator += (const TVector2& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		return *this;
	}

	inline TVector2& operator += (const T fScaler) {
		x += fScaler;
		y += fScaler;
		return *this;
	}

	inline TVector2& operator -= (const TVector2& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		return *this;
	}

	inline TVector2& operator -= (const T fScaler) {
		x -= fScaler;
		y -= fScaler;
		return *this;
	}

	inline TVector2& operator *= (const T fScalar) {
		x *= fScalar;
		y *= fScalar;
		return *this;
	}

	inline TVector2& operator *= (const TVector2& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		return *this;
	}

	inline TVector2& operator /= (const T fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		return *this;
	}

	inline TVector2& operator /= (const TVector2& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		return *this;
	}

	inline bool operator < (const TVector2& rhs) const {
		return (x < rhs.x && y < rhs.y);
	}

	inline bool operator > (const TVector2& rhs) const {
		return (x > rhs.x && y > rhs.y);
	}

public:
	inline T length(void) const {
		return MathUtil::sqrt(x * x + y * y);
	}

	inline T squaredLength(void) const {
		return x * x + y * y;
	}

	inline T distance(const TVector2& rhs) const {
		return (*this - rhs).length();
	}

	inline T squaredDistance(const TVector2& rhs) const {
		return (*this - rhs).squaredLength();
	}

	inline T dotProduct(const TVector2& vec) const {
		return x * vec.x + y * vec.y;
	}

	inline T crossProduct(const TVector2& rkVector) const {
		return x * rkVector.y - y * rkVector.x;
	}

	inline TVector2& normalise(void) {
		T fLength = length();
		if (fLength > 0.0f) {
			T invLength = T(1.0) / fLength;
			x *= invLength;
			y *= invLength;
		}

		return *this;
	}

	inline TVector2& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);

		return *this;
	}
public:
	// special points
	static const TVector2 ZERO;
	static const TVector2 ONE;
	static const TVector2 UNIT_X;
	static const TVector2 UNIT_Y;
	static const TVector2 NEGATIVE_UNIT_X;
	static const TVector2 NEGATIVE_UNIT_Y;
	static const TVector2 UNIT_SCALE;
};


typedef TVector2<float> fVector2;

}