#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

class Vector3
{
public:
	float x, y, z;

public:
	// Construct
	Vector3() : x(0.f), y(0.f), z(0.f) { }
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z){ }
	Vector3(const Vector2& _xy, float _z) : x(_xy.x), y(_xy.y), z(_z) { }
	Vector3(float _x, const Vector2& _yz) : x(_x), y(_yz.x), z(_yz.y) { }
	inline explicit Vector3(const float s) : x(s), y(s), z(s) { }
	inline explicit Vector3(const float f[3]) : x(f[0]), y(f[1]), z(f[2]) { }
	inline explicit Vector3(float* const f) : x(f[0]), y(f[1]), z(f[2]) { }
	inline explicit Vector3(const int f[3]) : x((float)f[0]), y((float)f[1]), z((float)f[2]) { }

public:
	inline float operator [] (const size_t i) const {
		assert(i < 3);
		return *(&x + i);
	}

	inline float& operator [] (const size_t i) {
		assert(i < 3);
		return *(&x + i);
	}

	inline float* ptr(void) {
		return &x;
	}

	inline const float* ptr(void) const {
		return &x;
	}

	inline Vector2 xy(void) const { return Vector2(x, y); }
	inline Vector2 xz(void) const { return Vector2(x, z); }
	inline Vector2 yz(void) const { return Vector2(y, z); }

public:
	// Operations
	inline Vector3& operator = (const Vector3& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;
		return *this;
	}

	inline Vector3& operator = (const float fScalar) {
		x = fScalar;
		y = fScalar;
		z = fScalar;
		return *this;
	}

	inline bool operator == (const Vector3& rkVector) const {
		return (x == rkVector.x && y == rkVector.y && z == rkVector.z);
	}

	inline bool operator != (const Vector3& rkVector) const {
		return (x != rkVector.x || y != rkVector.y || z != rkVector.z);
	}

public:
	// Arithmetic operations
	inline Vector3 operator + (const Vector3& rkVector) const {
		return Vector3( x + rkVector.x, y + rkVector.y, z + rkVector.z);
	}

	inline Vector3 operator - (const Vector3& rkVector) const {
		return Vector3( x - rkVector.x, y - rkVector.y, z - rkVector.z);
	}

	inline Vector3 operator * (const float fScalar) const {
		return Vector3( x * fScalar, y * fScalar, z * fScalar);
	}

	inline Vector3 operator * (const Vector3& rhs) const {
		return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	inline Vector3 operator / (const float fScalar) const {
		assert(fScalar != 0.0);
		return Vector3(x / fScalar, y / fScalar, z / fScalar);
	}

	inline Vector3 operator / (const Vector3& rhs) const {
		return Vector3( x / rhs.x, y / rhs.y, z / rhs.z);
	}

	inline const Vector3& operator + () const {
		return *this;
	}

	inline Vector3 operator - () const {
		return Vector3(-x, -y, -z);
	}

public:
	// overloaded operators to help Vector3
	inline friend Vector3 operator * (const float fScalar, const Vector3& rkVector) {
		return Vector3(fScalar * rkVector.x, fScalar * rkVector.y, fScalar * rkVector.z);
	}

	inline friend Vector3 operator / (const float fScalar, const Vector3& rkVector) {
		return Vector3( fScalar / rkVector.x, fScalar / rkVector.y, fScalar / rkVector.z);
	}

	inline friend Vector3 operator + (const Vector3& lhs, const float rhs) {
		return Vector3( lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
	}

	inline friend Vector3 operator + (const float lhs, const Vector3& rhs) {
		return Vector3( lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
	}

	inline friend Vector3 operator - (const Vector3& lhs, const float rhs) {
		return Vector3( lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
	}

	inline friend Vector3 operator - (const float lhs, const Vector3& rhs) {
		return Vector3( lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
	}

	inline friend Vector3 reflect(const Vector3& incident, const Vector3& normal) {
		return incident - 2.f * normal.dotProduct(incident) * normal;
	}
public:
	// arithmetic updates
	inline Vector3& operator += (const Vector3& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;
		return *this;
	}

	inline Vector3& operator += (const float fScaler) {
		x += fScaler;
		y += fScaler;
		z += fScaler;
		return *this;
	}

	inline Vector3& operator -= (const Vector3& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;
		return *this;
	}

	inline Vector3& operator -= (const float fScaler) {
		x -= fScaler;
		y -= fScaler;
		z -= fScaler;
		return *this;
	}

	inline Vector3& operator *= (const float fScalar) {
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
		return *this;
	}

	inline Vector3& operator *= (const Vector3& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;
		return *this;
	}

	inline Vector3& operator /= (const float fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		z /= fScalar;
		return *this;
	}

	inline Vector3& operator /= (const Vector3& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;
		return *this;
	}

	inline bool operator < (const Vector3& rhs) const {
		return (x < rhs.x && y < rhs.y && z < rhs.z);
	}

	inline bool operator > (const Vector3& rhs) const {
		return (x > rhs.x && y > rhs.y && z > rhs.z);
	}

public:
	inline float length(void) const {
		return MathUtil::sqrt(x * x + y * y + z * z);
	}

	inline float squaredLength(void) const {
		return x * x + y * y + z * z;
	}

	inline float distance(const Vector3& rhs) const {
		return (*this - rhs).length();
	}

	inline float squaredDistance(const Vector3& rhs) const {
		return (*this - rhs).squaredLength();
	}

	inline float dotProduct(const Vector3& vec) const {
		return x * vec.x + y * vec.y + z * vec.z;
	}

	inline Vector3 crossProduct(const Vector3& rkVector) const {
		return Vector3(
			y * rkVector.z - z * rkVector.y,
			z * rkVector.x - x * rkVector.z,
			x * rkVector.y - y * rkVector.x);
	}

	inline Vector3& normalise(void) {
		float fLength = length();
		if (fLength > 0.0f) {
			float invLength = 1.f / fLength;
			x *= invLength;
			y *= invLength;
			z *= invLength;
		}

		return *this;
	}

	inline Vector3& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);
		z = MathUtil::saturate(z);

		return *this;
	}

public:
	// special points
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
	static const Vector3 NEGATIVE_UNIT_X;
	static const Vector3 NEGATIVE_UNIT_Y;
	static const Vector3 NEGATIVE_UNIT_Z;
	static const Vector3 UNIT_SCALE;
	static const Vector3 WHITE;
	static const Vector3 BLACK;
	static const Vector3 RED;
	static const Vector3 GREEN;
	static const Vector3 BLUE;
	static const Vector3 PURPLE;
	static const Vector3 GRAY;
};

}