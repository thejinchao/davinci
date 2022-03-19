#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

class Vector2
{
public:
	float x, y;

public:
	// Construct
	Vector2() : x(0.f), y(0.f) { }
	Vector2(float _x, float _y) : x(_x), y(_y){ }
	inline explicit Vector2(const float s) : x(s), y(s) { }
	inline explicit Vector2(const float f[2]) : x(f[0]), y(f[1]) { }
	inline explicit Vector2(float* const f) : x(f[0]), y(f[1]) { }
	inline explicit Vector2(const int f[2]) : x((float)f[0]), y((float)f[1]) { }

public:
	inline float operator [] (const size_t i) const {
		assert(i < 2);
		return *(&x + i);
	}

	inline float& operator [] (const size_t i) {
		assert(i < 2);
		return *(&x + i);
	}

	inline float* ptr(void) {
		return &x;
	}

	inline const float* ptr(void) const {
		return &x;
	}
public:
	// Operations
	inline Vector2& operator = (const Vector2& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		return *this;
	}

	inline Vector2& operator = (const float fScalar) {
		x = fScalar;
		y = fScalar;
		return *this;
	}

	inline bool operator == (const Vector2& rkVector) const {
		return (x == rkVector.x && y == rkVector.y);
	}

	inline bool operator != (const Vector2& rkVector) const {
		return (x != rkVector.x || y != rkVector.y);
	}

public:
	// Arithmetic operations
	inline Vector2 operator + (const Vector2& rkVector) const {
		return Vector2( x + rkVector.x, y + rkVector.y);
	}

	inline Vector2 operator - (const Vector2& rkVector) const {
		return Vector2( x - rkVector.x, y - rkVector.y);
	}

	inline Vector2 operator * (const float fScalar) const {
		return Vector2( x * fScalar, y * fScalar);
	}

	inline Vector2 operator * (const Vector2& rhs) const {
		return Vector2(x * rhs.x, y * rhs.y);
	}

	inline Vector2 operator / (const float fScalar) const {
		assert(fScalar != 0.0);
		return Vector2(x / fScalar, y / fScalar);
	}

	inline Vector2 operator / (const Vector2& rhs) const {
		return Vector2( x / rhs.x, y / rhs.y);
	}

	inline const Vector2& operator + () const {
		return *this;
	}

	inline Vector2 operator - () const {
		return Vector2(-x, -y);
	}

public:
	// overloaded operators to help Vector2
	inline friend Vector2 operator * (const float fScalar, const Vector2& rkVector) {
		return Vector2(fScalar * rkVector.x, fScalar * rkVector.y);
	}

	inline friend Vector2 operator / (const float fScalar, const Vector2& rkVector) {
		return Vector2( fScalar / rkVector.x, fScalar / rkVector.y);
	}

	inline friend Vector2 operator + (const Vector2& lhs, const float rhs) {
		return Vector2( lhs.x + rhs, lhs.y + rhs);
	}

	inline friend Vector2 operator + (const float lhs, const Vector2& rhs) {
		return Vector2( lhs + rhs.x, lhs + rhs.y);
	}

	inline friend Vector2 operator - (const Vector2& lhs, const float rhs) {
		return Vector2( lhs.x - rhs, lhs.y - rhs);
	}

	inline friend Vector2 operator - (const float lhs, const Vector2& rhs) {
		return Vector2( lhs - rhs.x, lhs - rhs.y);
	}

public:
	// arithmetic updates
	inline Vector2& operator += (const Vector2& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		return *this;
	}

	inline Vector2& operator += (const float fScaler) {
		x += fScaler;
		y += fScaler;
		return *this;
	}

	inline Vector2& operator -= (const Vector2& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		return *this;
	}

	inline Vector2& operator -= (const float fScaler) {
		x -= fScaler;
		y -= fScaler;
		return *this;
	}

	inline Vector2& operator *= (const float fScalar) {
		x *= fScalar;
		y *= fScalar;
		return *this;
	}

	inline Vector2& operator *= (const Vector2& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		return *this;
	}

	inline Vector2& operator /= (const float fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		return *this;
	}

	inline Vector2& operator /= (const Vector2& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		return *this;
	}

	inline bool operator < (const Vector2& rhs) const {
		return (x < rhs.x && y < rhs.y);
	}

	inline bool operator > (const Vector2& rhs) const {
		return (x > rhs.x && y > rhs.y);
	}

public:
	inline float length(void) const {
		return MathUtil::sqrt(x * x + y * y);
	}

	inline float squaredLength(void) const {
		return x * x + y * y;
	}

	inline float distance(const Vector2& rhs) const {
		return (*this - rhs).length();
	}

	inline float squaredDistance(const Vector2& rhs) const {
		return (*this - rhs).squaredLength();
	}

	inline float dotProduct(const Vector2& vec) const {
		return x * vec.x + y * vec.y;
	}

	inline float crossProduct(const Vector2& rkVector) const {
		return x * rkVector.y - y * rkVector.x;
	}

	inline Vector2& normalise(void) {
		float fLength = length();
		if (fLength > 0.0f) {
			float invLength = 1.f / fLength;
			x *= invLength;
			y *= invLength;
		}

		return *this;
	}

	inline Vector2& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);

		return *this;
	}
public:
	// special points
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UNIT_X;
	static const Vector2 UNIT_Y;
	static const Vector2 NEGATIVE_UNIT_X;
	static const Vector2 NEGATIVE_UNIT_Y;
	static const Vector2 UNIT_SCALE;
};

}