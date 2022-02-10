#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

class Vector4
{
public:
	float x, y, z, w;

public:
	// Construct
	Vector4() : x(0.f), y(0.f), z(0.f), w(0.f) { }
	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w){ }
	Vector4(const Vector3& _xyz, float _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) { }
	inline explicit Vector4(const float s) : x(s), y(s), z(s), w(s) { }
	inline explicit Vector4(const float f[4]) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) { }
	inline explicit Vector4(float* const f) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) { }
	inline explicit Vector4(const int f[4]) : x((float)f[0]), y((float)f[1]), z((float)f[2]), w((float)f[3]) { }

public:
	inline float operator [] (const size_t i) const {
		assert(i < 4);
		return *(&x + i);
	}

	inline float& operator [] (const size_t i) {
		assert(i < 4);
		return *(&x + i);
	}

	inline float* ptr(void) {
		return &x;
	}

	inline const float* ptr(void) const {
		return &x;
	}

	inline Vector3 xyz(void) const { return Vector3(x, y, z); }

public:
	// Operations
	inline Vector4& operator = (const Vector4& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;
		w = rkVector.w;
		return *this;
	}

	inline Vector4& operator = (const float fScalar) {
		x = fScalar;
		y = fScalar;
		z = fScalar;
		w = fScalar;
		return *this;
	}

	inline bool operator == (const Vector4& rkVector) const {
		return (x == rkVector.x && y == rkVector.y && z == rkVector.z && w == rkVector.w);
	}

	inline bool operator != (const Vector4& rkVector) const {
		return (x != rkVector.x || y != rkVector.y || z != rkVector.z || w != rkVector.w);
	}

public:
	// Arithmetic operations
	inline Vector4 operator + (const Vector4& rkVector) const {
		return Vector4( x + rkVector.x, y + rkVector.y, z + rkVector.z, w + rkVector.w);
	}

	inline Vector4 operator - (const Vector4& rkVector) const {
		return Vector4( x - rkVector.x, y - rkVector.y, z - rkVector.z, w - rkVector.w);
	}

	inline Vector4 operator * (const float fScalar) const {
		return Vector4( x * fScalar, y * fScalar, z * fScalar, w * fScalar);
	}

	inline Vector4 operator * (const Vector4& rhs) const {
		return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
	}

	inline Vector4 operator / (const float fScalar) const {
		assert(fScalar != 0.0);
		return Vector4(x / fScalar, y / fScalar, z / fScalar, w / fScalar);
	}

	inline Vector4 operator / (const Vector4& rhs) const {
		return Vector4( x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
	}

	inline const Vector4& operator + () const {
		return *this;
	}

	inline Vector4 operator - () const {
		return Vector4(-x, -y, -z, -w);
	}

public:
	// overloaded operators to help Vector4
	inline friend Vector4 operator * (const float fScalar, const Vector4& rkVector) {
		return Vector4(fScalar * rkVector.x, fScalar * rkVector.y, fScalar * rkVector.z, fScalar * rkVector.w);
	}

	inline friend Vector4 operator / (const float fScalar, const Vector4& rkVector) {
		return Vector4( fScalar / rkVector.x, fScalar / rkVector.y, fScalar / rkVector.z, fScalar / rkVector.w);
	}

	inline friend Vector4 operator + (const Vector4& lhs, const float rhs) {
		return Vector4( lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
	}

	inline friend Vector4 operator + (const float lhs, const Vector4& rhs) {
		return Vector4( lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
	}

	inline friend Vector4 operator - (const Vector4& lhs, const float rhs) {
		return Vector4( lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
	}

	inline friend Vector4 operator - (const float lhs, const Vector4& rhs) {
		return Vector4( lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.z);
	}

public:
	// arithmetic updates
	inline Vector4& operator += (const Vector4& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;
		w += rkVector.w;
		return *this;
	}

	inline Vector4& operator += (const float fScaler) {
		x += fScaler;
		y += fScaler;
		z += fScaler;
		w += fScaler;
		return *this;
	}

	inline Vector4& operator -= (const Vector4& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;
		w -= rkVector.w;
		return *this;
	}

	inline Vector4& operator -= (const float fScaler) {
		x -= fScaler;
		y -= fScaler;
		z -= fScaler;
		w -= fScaler;
		return *this;
	}

	inline Vector4& operator *= (const float fScalar) {
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
		w *= fScalar;
		return *this;
	}

	inline Vector4& operator *= (const Vector4& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;
		w *= rkVector.w;
		return *this;
	}

	inline Vector4& operator /= (const float fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		z /= fScalar;
		w /= fScalar;
		return *this;
	}

	inline Vector4& operator /= (const Vector4& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;
		w /= rkVector.w;
		return *this;
	}

	inline bool operator < (const Vector4& rhs) const {
		return (x < rhs.x && y < rhs.y && z < rhs.z && w < rhs.w);
	}

	inline bool operator > (const Vector4& rhs) const {
		return (x > rhs.x && y > rhs.y && z > rhs.z && w > rhs.w);
	}

public:

	inline float dotProduct(const Vector4& vec) const {
		return (x * vec.x + y * vec.y) + (z * vec.z + w * vec.w);
	}

	inline Vector4& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);
		z = MathUtil::saturate(z);
		w = MathUtil::saturate(w);

		return *this;
	}
public:
	// special points
	static const Vector4 ZERO;
	static const Vector4 WHITE;
	static const Vector4 BLACK;
	static const Vector4 RED;
	static const Vector4 GREEN;
	static const Vector4 BLUE;
	static const Vector4 PURPLE;
	static const Vector4 GRAY;
};

}