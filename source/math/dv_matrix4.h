#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

template<typename T>
class TMatrix4
{
protected:
	/* 
        | m[0][0]  m[0][1]  m[0][2]  m[0][3] |
        | m[1][0]  m[1][1]  m[1][2]  m[1][3] |
		| m[2][0]  m[2][1]  m[2][2]  m[2][3] |
		| m[3][0]  m[3][1]  m[3][2]  m[3][3] |
	*/
	union {
		T m[4][4];
		T _m[16];
	};
public:
	// constructor, It does NOT initialize the matrix for efficiency.
	TMatrix4() {}
	inline explicit TMatrix4(const T arr[4][4]) {
		memcpy(m, arr, 16 * sizeof(T));
	}
	inline TMatrix4(const TMatrix4& rkMatrix) {
		memcpy(m, rkMatrix.m, 16 * sizeof(T));
	}
	TMatrix4(
		T m00, T m01, T m02, T m03,
		T m10, T m11, T m12, T m13,
		T m20, T m21, T m22, T m23,
		T m30, T m31, T m32, T m33)
	{
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
		m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
	}

public:
	inline const T* operator[] (size_t iRow) const {
		assert(iRow < 4);
		return m[iRow];
	}

	inline T* operator[] (size_t iRow) {
		assert(iRow < 4);
		return m[iRow];
	}

public:
	// Operators
	inline TMatrix4& operator = (const TMatrix4& rkMatrix) {
		memcpy(m, rkMatrix.m, 16 * sizeof(T));
		return *this;
	}

	bool operator == (const TMatrix4& rkMatrix) const;
	inline bool operator != (const TMatrix4& rkMatrix) const {
		return !operator==(rkMatrix);
	}

	// return this + m2
	TMatrix4 operator + (const TMatrix4 &m2) const;
	// return this - m2
	TMatrix4 operator - (const TMatrix4 &m2) const;
	//return this * m2
	TMatrix4 operator * (const TMatrix4 &m2) const;
	//return this * scalar
	inline TMatrix4 operator*(T scalar) const {
		return TMatrix4(
			scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
			scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
			scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
			scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
	}

	/// return scalar * matrix
	friend TMatrix4 operator* (T scalar, const TMatrix4& rkMatrix) {
		return TMatrix4(
			scalar*rkMatrix[0][0], scalar*rkMatrix[0][1], scalar*rkMatrix[0][2], scalar*rkMatrix[0][3],
			scalar*rkMatrix[1][0], scalar*rkMatrix[1][1], scalar*rkMatrix[1][2], scalar*rkMatrix[1][3],
			scalar*rkMatrix[2][0], scalar*rkMatrix[2][1], scalar*rkMatrix[2][2], scalar*rkMatrix[2][3],
			scalar*rkMatrix[3][0], scalar*rkMatrix[3][1], scalar*rkMatrix[3][2], scalar*rkMatrix[3][3]);
	}

	// return -this
	TMatrix4 operator - () const	{
		return TMatrix4(
			-m[0][0], -m[0][1], -m[0][2], -m[0][3],
			-m[1][0], -m[1][1], -m[1][2], -m[1][3],
			-m[2][0], -m[2][1], -m[2][2], -m[2][3],
			-m[3][0], -m[3][1], -m[3][2], -m[3][3]);
	}

	/*  return Matrix * fVector4
          | m[0][0]  m[0][1]  m[0][2]  m[0][3] |   |x|
          | m[1][0]  m[1][1]  m[1][2]  m[1][3] | * |y|
          | m[2][0]  m[2][1]  m[2][2]  m[2][3] |   |z|
          | m[3][0]  m[3][1]  m[3][2]  m[3][3] |   |w|
	*/	
	inline TVector4<T> operator * (const TVector4<T>& v) const {
		return TVector4<T>(
			(m[0][0] * v.x + m[0][1] * v.y) + (m[0][2] * v.z + m[0][3] * v.w),
			(m[1][0] * v.x + m[1][1] * v.y) + (m[1][2] * v.z + m[1][3] * v.w),
			(m[2][0] * v.x + m[2][1] * v.y) + (m[2][2] * v.z + m[2][3] * v.w),
			(m[3][0] * v.x + m[3][1] * v.y) + (m[3][2] * v.z + m[3][3] * v.w)
		);
	}

	/*
		Transforms the given 3-D vector by the matrix, projecting the
		result back into <i>w</i> = 1.
		@note
		This means that the initial <i>w</i> is considered to be 1.0,
		and then all the tree elements of the resulting 3-D vector are
		divided by the resulting <i>w</i>.
	*/
	inline TVector3<T> operator * (const TVector3<T> &v) const {
		TVector3<T> r;
		T fInvW = 1.0f / (m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]);

		r.x = (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3]) * fInvW;
		r.y = (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3]) * fInvW;
		r.z = (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]) * fInvW;
		return r;
	}

	/* return fVector4 * Matrix
                 | m[0][0]  m[0][1]  m[0][2]  m[0][3] |
     [x,y,z,w] * | m[1][0]  m[1][1]  m[1][2]  m[1][3] |
                 | m[2][0]  m[2][1]  m[2][2]  m[2][3] |
                 | m[3][0]  m[3][1]  m[3][2]  m[3][3] |
	*/	
	friend TVector4<T> operator * (const TVector4<T>& v, const TMatrix4& mat) {
		return TVector4<T>(
			(v.x*mat[0][0] + v.y*mat[1][0]) + (v.z*mat[2][0] + v.w*mat[3][0]),
			(v.x*mat[0][1] + v.y*mat[1][1]) + (v.z*mat[2][1] + v.w*mat[3][1]),
			(v.x*mat[0][2] + v.y*mat[1][2]) + (v.z*mat[2][2] + v.w*mat[3][2]),
			(v.x*mat[0][3] + v.y*mat[1][3]) + (v.z*mat[2][3] + v.w*mat[3][3])
		);
	}

	friend TVector3<T> operator * (const TVector3<T>& v, const TMatrix4& mat) {
		TVector3<T> r;
		T fInvW = 1.0f / (mat.m[0][3] * v.x + mat.m[1][3] * v.y + mat.m[2][3] * v.z + mat.m[3][3]);

		r.x = (mat.m[0][0] * v.x + mat.m[1][0] * v.y + mat.m[2][0] * v.z + mat.m[3][0]) * fInvW;
		r.y = (mat.m[0][1] * v.x + mat.m[1][1] * v.y + mat.m[2][1] * v.z + mat.m[3][1]) * fInvW;
		r.z = (mat.m[0][2] * v.x + mat.m[1][2] * v.y + mat.m[2][2] * v.z + mat.m[3][2]) * fInvW;
		return r;
	}
public:
	static inline TMatrix4 makeTrans(T x, T y, T z) {
		return TMatrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, z, 1);
	}
	
	static inline TMatrix4 makeTrans(const TVector3<T> pos) {
		return TMatrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			pos.x, pos.y, pos.z, 1);
	}

	static inline TMatrix4 makeScale(T x, T y, T z) {
		return TMatrix4(
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1);
	}

	static inline TMatrix4 makeRotate_X(T angle) {
		T s = sinf(angle);
		T c = cosf(angle);
		return TMatrix4(
			1, 0, 0, 0,
			0, c, s, 0,
			0, -s, c, 0,
			0, 0, 0, 1);
	}

	static inline TMatrix4 makeRotate_Y(T angle) {
		T s = sinf(angle);
		T c = cosf(angle);
		return TMatrix4(
			c, 0, s, 0,
			0, 1, 0, 0,
			-s, 0, c, 0,
			0, 0, 0, 1);
	}

	static inline TMatrix4 makeRotate_Z(T angle) {
		T s = sinf(angle);
		T c = cosf(angle);
		return TMatrix4(
			c, s, 0, 0,
			-s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	inline TMatrix4 transpose(void) const {
		return TMatrix4(
			m[0][0], m[1][0], m[2][0], m[3][0],
			m[0][1], m[1][1], m[2][1], m[3][1],
			m[0][2], m[1][2], m[2][2], m[3][2],
			m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	//return this^{-1}, return ZERO if no solution
	TMatrix4 inverse(void) const;

	//return a left-handed, look-at matrix.
	static TMatrix4 lookatLH(const TVector3<T>& eye, const TVector3<T>& lookat, const TVector3<T>& up);
	//return a left-handed perspective projection matrix based on a field of view.
	static TMatrix4 perspectiveFovLH(T fov, T aspect, T zNear, T zFar);

public:
	static const TMatrix4 ZERO;
	static const TMatrix4 IDENTITY;

};

//-------------------------------------------------------------------------------------
template<typename T>
bool TMatrix4<T>::operator == (const TMatrix4<T>& m2) const {
	if (
		m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
		m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
		m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
		m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3])
		return false;
	return true;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::operator * (const TMatrix4<T>& m2) const
{
	TMatrix4<T> r;
	r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0] + m[0][3] * m2.m[3][0];
	r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1] + m[0][3] * m2.m[3][1];
	r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2] + m[0][3] * m2.m[3][2];
	r.m[0][3] = m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3] * m2.m[3][3];

	r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0] + m[1][3] * m2.m[3][0];
	r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1] + m[1][3] * m2.m[3][1];
	r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2] + m[1][3] * m2.m[3][2];
	r.m[1][3] = m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3] * m2.m[3][3];

	r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0] + m[2][3] * m2.m[3][0];
	r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1] + m[2][3] * m2.m[3][1];
	r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2] + m[2][3] * m2.m[3][2];
	r.m[2][3] = m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3] * m2.m[3][3];

	r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
	r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
	r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
	r.m[3][3] = m[3][0] * m2.m[0][3] + m[3][1] * m2.m[1][3] + m[3][2] * m2.m[2][3] + m[3][3] * m2.m[3][3];

	return r;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::operator + (const TMatrix4<T>& m2) const
{
	TMatrix4<T> r;

	r.m[0][0] = m[0][0] + m2.m[0][0];
	r.m[0][1] = m[0][1] + m2.m[0][1];
	r.m[0][2] = m[0][2] + m2.m[0][2];
	r.m[0][3] = m[0][3] + m2.m[0][3];

	r.m[1][0] = m[1][0] + m2.m[1][0];
	r.m[1][1] = m[1][1] + m2.m[1][1];
	r.m[1][2] = m[1][2] + m2.m[1][2];
	r.m[1][3] = m[1][3] + m2.m[1][3];

	r.m[2][0] = m[2][0] + m2.m[2][0];
	r.m[2][1] = m[2][1] + m2.m[2][1];
	r.m[2][2] = m[2][2] + m2.m[2][2];
	r.m[2][3] = m[2][3] + m2.m[2][3];

	r.m[3][0] = m[3][0] + m2.m[3][0];
	r.m[3][1] = m[3][1] + m2.m[3][1];
	r.m[3][2] = m[3][2] + m2.m[3][2];
	r.m[3][3] = m[3][3] + m2.m[3][3];

	return r;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::operator - (const TMatrix4<T>& m2) const
{
	TMatrix4<T> r;
	r.m[0][0] = m[0][0] - m2.m[0][0];
	r.m[0][1] = m[0][1] - m2.m[0][1];
	r.m[0][2] = m[0][2] - m2.m[0][2];
	r.m[0][3] = m[0][3] - m2.m[0][3];

	r.m[1][0] = m[1][0] - m2.m[1][0];
	r.m[1][1] = m[1][1] - m2.m[1][1];
	r.m[1][2] = m[1][2] - m2.m[1][2];
	r.m[1][3] = m[1][3] - m2.m[1][3];

	r.m[2][0] = m[2][0] - m2.m[2][0];
	r.m[2][1] = m[2][1] - m2.m[2][1];
	r.m[2][2] = m[2][2] - m2.m[2][2];
	r.m[2][3] = m[2][3] - m2.m[2][3];

	r.m[3][0] = m[3][0] - m2.m[3][0];
	r.m[3][1] = m[3][1] - m2.m[3][1];
	r.m[3][2] = m[3][2] - m2.m[3][2];
	r.m[3][3] = m[3][3] - m2.m[3][3];

	return r;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::inverse(void) const
{
	T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	T m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	T v0 = m20 * m31 - m21 * m30;
	T v1 = m20 * m32 - m22 * m30;
	T v2 = m20 * m33 - m23 * m30;
	T v3 = m21 * m32 - m22 * m31;
	T v4 = m21 * m33 - m23 * m31;
	T v5 = m22 * m33 - m23 * m32;

	T t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
	T t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
	T t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
	T t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

	T invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	T d00 = t00 * invDet;
	T d10 = t10 * invDet;
	T d20 = t20 * invDet;
	T d30 = t30 * invDet;

	T d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	T d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	T d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	T d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	T d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	T d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	return TMatrix4<T>(
		d00, d01, d02, d03,
		d10, d11, d12, d13,
		d20, d21, d22, d23,
		d30, d31, d32, d33);
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::lookatLH(const TVector3<T>& eye, const TVector3<T>& lookat, const TVector3<T>& up)
{
	assert(lookat != eye);
	assert(up != TVector3<T>::ZERO);

	TVector3<T> zaxis = (lookat - eye).normalise();
	TVector3<T> xaxis = up.crossProduct(zaxis).normalise();
	TVector3<T> yaxis = zaxis.crossProduct(xaxis).normalise();

	return TMatrix4<T>(
		xaxis.x, yaxis.x, zaxis.x, 0.f,
		xaxis.y, yaxis.y, zaxis.y, 0.f,
		xaxis.z, yaxis.z, zaxis.z, 0.f,
		-xaxis.dotProduct(eye), -yaxis.dotProduct(eye), -zaxis.dotProduct(eye), 1.f);
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix4<T> TMatrix4<T>::perspectiveFovLH(T fov, T aspect, T zNear, T zFar)
{
	assert(zNear > 0.f && zFar > 0.f);
	assert(!MathUtil::floatEqual(fov, 0.f, 0.00001f * 2));
	assert(!MathUtil::floatEqual(aspect, 0.f, 0.00001f));
	assert(!MathUtil::floatEqual(zNear, zFar, 0.00001f));

	T sinFov = sinf(fov * 0.5f);
	T cosFov = cosf(fov * 0.5f);

	T height = cosFov / sinFov;
	T width = height / aspect;
	T range = zFar / (zFar - zNear);

	return TMatrix4<T>(
		width, 0.f, 0.f, 0.f,
		0.f, height, 0.f, 0.f,
		0.f, 0.f, range, 1.f,
		0.f, 0.f, -range * zNear, 0.f);
}

typedef TMatrix4<float> fMatrix4;

}
