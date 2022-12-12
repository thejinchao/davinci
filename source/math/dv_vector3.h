#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

template<typename T>
class TVector3
{
public:
	T x, y, z;

public:
	// Construct
	TVector3() : x(0.f), y(0.f), z(0.f) { }
	TVector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z){ }
	TVector3(const TVector2<T>& _xy, T _z) : x(_xy.x), y(_xy.y), z(_z) { }
	TVector3(T _x, const TVector2<T>& _yz) : x(_x), y(_yz.x), z(_yz.y) { }
	inline explicit TVector3(const T s) : x(s), y(s), z(s) { }
	inline explicit TVector3(const T f[3]) : x(f[0]), y(f[1]), z(f[2]) { }
	inline explicit TVector3(T* const f) : x(f[0]), y(f[1]), z(f[2]) { }
	inline explicit TVector3(const int f[3]) : x((T)f[0]), y((T)f[1]), z((T)f[2]) { }

public:
	inline T operator [] (const size_t i) const {
		assert(i < 3);
		return *(&x + i);
	}

	inline T& operator [] (const size_t i) {
		assert(i < 3);
		return *(&x + i);
	}

	inline T* ptr(void) {
		return &x;
	}

	inline const T* ptr(void) const {
		return &x;
	}

	inline TVector2<T> xy(void) const { return TVector2<T>(x, y); }
	inline TVector2<T> xz(void) const { return TVector2<T>(x, z); }
	inline TVector2<T> yz(void) const { return TVector2<T>(y, z); }

public:
	// Operations
	inline TVector3& operator = (const TVector3& rkVector) {
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;
		return *this;
	}

	inline TVector3& operator = (const T fScalar) {
		x = fScalar;
		y = fScalar;
		z = fScalar;
		return *this;
	}

	inline bool operator == (const TVector3& rkVector) const {
		return (x == rkVector.x && y == rkVector.y && z == rkVector.z);
	}

	inline bool operator != (const TVector3& rkVector) const {
		return (x != rkVector.x || y != rkVector.y || z != rkVector.z);
	}

public:
	// Arithmetic operations
	inline TVector3 operator + (const TVector3& rkVector) const {
		return TVector3( x + rkVector.x, y + rkVector.y, z + rkVector.z);
	}

	inline TVector3 operator - (const TVector3& rkVector) const {
		return TVector3( x - rkVector.x, y - rkVector.y, z - rkVector.z);
	}

	inline TVector3 operator * (const T fScalar) const {
		return TVector3( x * fScalar, y * fScalar, z * fScalar);
	}

	inline TVector3 operator * (const TVector3& rhs) const {
		return TVector3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	inline TVector3 operator / (const T fScalar) const {
		assert(fScalar != 0.0);
		return TVector3(x / fScalar, y / fScalar, z / fScalar);
	}

	inline TVector3 operator / (const TVector3& rhs) const {
		return TVector3( x / rhs.x, y / rhs.y, z / rhs.z);
	}

	inline const TVector3& operator + () const {
		return *this;
	}

	inline TVector3 operator - () const {
		return TVector3(-x, -y, -z);
	}

public:
	// overloaded operators to help TVector3
	inline friend TVector3 operator * (const T fScalar, const TVector3& rkVector) {
		return TVector3(fScalar * rkVector.x, fScalar * rkVector.y, fScalar * rkVector.z);
	}

	inline friend TVector3 operator / (const T fScalar, const TVector3& rkVector) {
		return TVector3( fScalar / rkVector.x, fScalar / rkVector.y, fScalar / rkVector.z);
	}

	inline friend TVector3 operator + (const TVector3& lhs, const T rhs) {
		return TVector3( lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
	}

	inline friend TVector3 operator + (const T lhs, const TVector3& rhs) {
		return TVector3( lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
	}

	inline friend TVector3 operator - (const TVector3& lhs, const T rhs) {
		return TVector3( lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
	}

	inline friend TVector3 operator - (const T lhs, const TVector3& rhs) {
		return TVector3( lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
	}

	inline friend TVector3 reflect(const TVector3& incident, const TVector3& normal) {
		return incident - 2.f * normal.dotProduct(incident) * normal;
	}
public:
	// arithmetic updates
	inline TVector3& operator += (const TVector3& rkVector) {
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;
		return *this;
	}

	inline TVector3& operator += (const T fScaler) {
		x += fScaler;
		y += fScaler;
		z += fScaler;
		return *this;
	}

	inline TVector3& operator -= (const TVector3& rkVector) {
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;
		return *this;
	}

	inline TVector3& operator -= (const T fScaler) {
		x -= fScaler;
		y -= fScaler;
		z -= fScaler;
		return *this;
	}

	inline TVector3& operator *= (const T fScalar) {
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
		return *this;
	}

	inline TVector3& operator *= (const TVector3& rkVector) {
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;
		return *this;
	}

	inline TVector3& operator /= (const T fScalar) {
		assert(fScalar != 0.0);

		x /= fScalar;
		y /= fScalar;
		z /= fScalar;
		return *this;
	}

	inline TVector3& operator /= (const TVector3& rkVector) {
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;
		return *this;
	}

	inline bool operator < (const TVector3& rhs) const {
		return (x < rhs.x && y < rhs.y && z < rhs.z);
	}

	inline bool operator > (const TVector3& rhs) const {
		return (x > rhs.x && y > rhs.y && z > rhs.z);
	}

public:
	inline T length(void) const {
		return MathUtil::sqrt(x * x + y * y + z * z);
	}

	inline T squaredLength(void) const {
		return x * x + y * y + z * z;
	}

	inline T distance(const TVector3& rhs) const {
		return (*this - rhs).length();
	}

	inline T squaredDistance(const TVector3& rhs) const {
		return (*this - rhs).squaredLength();
	}

	inline T dotProduct(const TVector3& vec) const {
		return x * vec.x + y * vec.y + z * vec.z;
	}

	inline TVector3 crossProduct(const TVector3& rkVector) const {
		return TVector3(
			y * rkVector.z - z * rkVector.y,
			z * rkVector.x - x * rkVector.z,
			x * rkVector.y - y * rkVector.x);
	}

	inline TVector3& normalise(void) {
		T fLength = length();
		if (fLength > 0.0f) {
			T invLength = 1.f / fLength;
			x *= invLength;
			y *= invLength;
			z *= invLength;
		}

		return *this;
	}

	inline TVector3& saturate(void) {
		x = MathUtil::saturate(x);
		y = MathUtil::saturate(y);
		z = MathUtil::saturate(z);

		return *this;
	}

public:
	// special points
	static const TVector3 ZERO;
	static const TVector3 ONE;
	static const TVector3 UNIT_X;
	static const TVector3 UNIT_Y;
	static const TVector3 UNIT_Z;
	static const TVector3 NEGATIVE_UNIT_X;
	static const TVector3 NEGATIVE_UNIT_Y;
	static const TVector3 NEGATIVE_UNIT_Z;
	static const TVector3 UNIT_SCALE;
	static const TVector3 WHITE;
	static const TVector3 BLACK;
	static const TVector3 RED;
	static const TVector3 GREEN;
	static const TVector3 BLUE;
	static const TVector3 PURPLE;
	static const TVector3 GRAY;
};

typedef TVector3<float> fVector3;

}