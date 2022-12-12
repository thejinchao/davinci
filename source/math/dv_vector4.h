#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

template<typename T>
class TVector4
{
public:
	T x, y, z, w;

public:
	// Construct
	TVector4() : x(0.f), y(0.f), z(0.f), w(0.f) { }
	TVector4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w){ }
	TVector4(const TVector3<T>& _xyz, T _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) { }
	inline explicit TVector4(const T s) : x(s), y(s), z(s), w(s) { }
	inline explicit TVector4(const T f[4]) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) { }
	inline explicit TVector4(T* const f) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) { }
	inline explicit TVector4(const int f[4]) : x((T)f[0]), y((T)f[1]), z((T)f[2]), w((T)f[3]) { }

public:
	inline T operator [] (const size_t i) const {
		assert(i < 4);
		return *(&x + i);
	}

	inline T& operator [] (const size_t i) {
		assert(i < 4);
		return *(&x + i);
	}

	inline T* ptr(void) {
		return &x;
	}

	inline const T* ptr(void) const {
		return &x;
	}

	inline TVector3<T> xyz(void) const { return TVector3<T>(x, y, z); }

public:
	// Operations
	inline TVector4& operator = (const TVector4& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;
		w = rkVector.w;
		return *this;
	}

	inline TVector4& operator = (const T fScalar) {
		x = fScalar;
		y = fScalar;
		z = fScalar;
		w = fScalar;
		return *this;
	}

	inline bool operator == (const TVector4& rkVector) const {
		return (x == rkVector.x && y == rkVector.y && z == rkVector.z && w == rkVector.w);
	}

	inline bool operator != (const TVector4& rkVector) const {
		return (x != rkVector.x || y != rkVector.y || z != rkVector.z || w != rkVector.w);
	}

public:
	// Arithmetic operations
	inline TVector4 operator + (const TVector4& rkVector) const {
		return TVector4( x + rkVector.x, y + rkVector.y, z + rkVector.z, w + rkVector.w);
	}

	inline TVector4 operator - (const TVector4& rkVector) const {
		return TVector4( x - rkVector.x, y - rkVector.y, z - rkVector.z, w - rkVector.w);
	}

	inline TVector4 operator * (const T fScalar) const {
		return TVector4( x * fScalar, y * fScalar, z * fScalar, w * fScalar);
	}

	inline TVector4 operator * (const TVector4& rhs) const {
		return TVector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
	}

	inline TVector4 operator / (const T fScalar) const {
		assert(fScalar != 0.0);
		return TVector4(x / fScalar, y / fScalar, z / fScalar, w / fScalar);
	}

	inline TVector4 operator / (const TVector4& rhs) const {
		return TVector4( x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
	}

	inline const TVector4& operator + () const {
		return *this;
	}

	inline TVector4 operator - () const {
		return TVector4(-x, -y, -z, -w);
	}

public:
	// overloaded operators to help TVector4
	inline friend TVector4 operator * (const T fScalar, const TVector4& rkVector) {
		return TVector4(fScalar * rkVector.x, fScalar * rkVector.y, fScalar * rkVector.z, fScalar * rkVector.w);
	}

	inline friend TVector4 operator / (const T fScalar, const TVector4& rkVector) {
		return TVector4( fScalar / rkVector.x, fScalar / rkVector.y, fScalar / rkVector.z, fScalar / rkVector.w);
	}

	inline friend TVector4 operator + (const TVector4& lhs, const T rhs) {
		return TVector4( lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
	}

	inline friend TVector4 operator + (const T lhs, const TVector4& rhs) {
		return TVector4( lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
	}

	inline friend TVector4 operator - (const TVector4& lhs, const T rhs) {
		return TVector4( lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
	}

	inline friend TVector4 operator - (const T lhs, const TVector4& rhs) {
		return TVector4( lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.z);
	}

public:
	// arithmetic updates
	inline TVector4& operator += (const TVector4& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;
		w += rkVector.w;
		return *this;
	}

	inline TVector4& operator += (const T fScaler) {
		x += fScaler;
		y += fScaler;
		z += fScaler;
		w += fScaler;
		return *this;
	}

	inline TVector4& operator -= (const TVector4& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;
		w -= rkVector.w;
		return *this;
	}

	inline TVector4& operator -= (const T fScaler) {
		x -= fScaler;
		y -= fScaler;
		z -= fScaler;
		w -= fScaler;
		return *this;
	}

	inline TVector4& operator *= (const T fScalar) {
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
		w *= fScalar;
		return *this;
	}

	inline TVector4& operator *= (const TVector4& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;
		w *= rkVector.w;
		return *this;
	}

	inline TVector4& operator /= (const T fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		z /= fScalar;
		w /= fScalar;
		return *this;
	}

	inline TVector4& operator /= (const TVector4& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;
		w /= rkVector.w;
		return *this;
	}

	inline bool operator < (const TVector4& rhs) const {
		return (x < rhs.x && y < rhs.y && z < rhs.z && w < rhs.w);
	}

	inline bool operator > (const TVector4& rhs) const {
		return (x > rhs.x && y > rhs.y && z > rhs.z && w > rhs.w);
	}

public:

	inline T dotProduct(const TVector4& vec) const {
		return (x * vec.x + y * vec.y) + (z * vec.z + w * vec.w);
	}

	inline TVector4& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);
		z = MathUtil::saturate(z);
		w = MathUtil::saturate(w);

		return *this;
	}
public:
	// special points
	static const TVector4 ZERO;
	static const TVector4 WHITE;
	static const TVector4 BLACK;
	static const TVector4 RED;
	static const TVector4 GREEN;
	static const TVector4 BLUE;
	static const TVector4 PURPLE;
	static const TVector4 GRAY;
};

typedef TVector4<float> fVector4;

}