#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

class Matrix4
{
protected:
	/* 
        | m[0][0]  m[0][1]  m[0][2]  m[0][3] |
        | m[1][0]  m[1][1]  m[1][2]  m[1][3] |
		| m[2][0]  m[2][1]  m[2][2]  m[2][3] |
		| m[3][0]  m[3][1]  m[3][2]  m[3][3] |
	*/
	union {
		float m[4][4];
		float _m[16];
	};
public:
	// constructor, It does NOT initialize the matrix for efficiency.
	Matrix4() {}
	inline explicit Matrix4(const float arr[4][4]) {
		memcpy(m, arr, 16 * sizeof(float));
	}
	inline Matrix4(const Matrix4& rkMatrix) {
		memcpy(m, rkMatrix.m, 16 * sizeof(float));
	}
	Matrix4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
	{
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
		m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
	}

public:
	inline const float* operator[] (size_t iRow) const {
		assert(iRow < 4);
		return m[iRow];
	}

	inline float* operator[] (size_t iRow) {
		assert(iRow < 4);
		return m[iRow];
	}

public:
	// Operators
	inline Matrix4& operator = (const Matrix4& rkMatrix) {
		memcpy(m, rkMatrix.m, 16 * sizeof(float));
		return *this;
	}

	bool operator == (const Matrix4& rkMatrix) const;
	inline bool operator != (const Matrix4& rkMatrix) const {
		return !operator==(rkMatrix);
	}

	// return this + m2
	Matrix4 operator + (const Matrix4 &m2) const;
	// return this - m2
	Matrix4 operator - (const Matrix4 &m2) const;
	//return this * m2
	Matrix4 operator * (const Matrix4 &m2) const;
	//return this * scalar
	inline Matrix4 operator*(float scalar) const {
		return Matrix4(
			scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
			scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
			scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
			scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
	}

	/// return scalar * matrix
	friend Matrix4 operator* (float scalar, const Matrix4& rkMatrix) {
		return Matrix4(
			scalar*rkMatrix[0][0], scalar*rkMatrix[0][1], scalar*rkMatrix[0][2], scalar*rkMatrix[0][3],
			scalar*rkMatrix[1][0], scalar*rkMatrix[1][1], scalar*rkMatrix[1][2], scalar*rkMatrix[1][3],
			scalar*rkMatrix[2][0], scalar*rkMatrix[2][1], scalar*rkMatrix[2][2], scalar*rkMatrix[2][3],
			scalar*rkMatrix[3][0], scalar*rkMatrix[3][1], scalar*rkMatrix[3][2], scalar*rkMatrix[3][3]);
	}

	// return -this
	Matrix4 operator - () const	{
		return Matrix4(
			-m[0][0], -m[0][1], -m[0][2], -m[0][3],
			-m[1][0], -m[1][1], -m[1][2], -m[1][3],
			-m[2][0], -m[2][1], -m[2][2], -m[2][3],
			-m[3][0], -m[3][1], -m[3][2], -m[3][3]);
	}

	/*  return Matrix * Vector4
          | m[0][0]  m[0][1]  m[0][2]  m[0][3] |   |x|
          | m[1][0]  m[1][1]  m[1][2]  m[1][3] | * |y|
          | m[2][0]  m[2][1]  m[2][2]  m[2][3] |   |z|
          | m[3][0]  m[3][1]  m[3][2]  m[3][3] |   |w|
	*/	
	inline Vector4 operator * (const Vector4& v) const {
		return Vector4(
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
	inline Vector3 operator * (const Vector3 &v) const {
		Vector3 r;
		float fInvW = 1.0f / (m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]);

		r.x = (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3]) * fInvW;
		r.y = (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3]) * fInvW;
		r.z = (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]) * fInvW;
		return r;
	}

	/* return Vector4 * Matrix
                 | m[0][0]  m[0][1]  m[0][2]  m[0][3] |
     [x,y,z,w] * | m[1][0]  m[1][1]  m[1][2]  m[1][3] |
                 | m[2][0]  m[2][1]  m[2][2]  m[2][3] |
                 | m[3][0]  m[3][1]  m[3][2]  m[3][3] |
	*/	
	friend Vector4 operator * (const Vector4& v, const Matrix4& mat) {
		return Vector4(
			(v.x*mat[0][0] + v.y*mat[1][0]) + (v.z*mat[2][0] + v.w*mat[3][0]),
			(v.x*mat[0][1] + v.y*mat[1][1]) + (v.z*mat[2][1] + v.w*mat[3][1]),
			(v.x*mat[0][2] + v.y*mat[1][2]) + (v.z*mat[2][2] + v.w*mat[3][2]),
			(v.x*mat[0][3] + v.y*mat[1][3]) + (v.z*mat[2][3] + v.w*mat[3][3])
		);
	}

	friend Vector3 operator * (const Vector3& v, const Matrix4& mat) {
		Vector3 r;
		float fInvW = 1.0f / (mat.m[0][3] * v.x + mat.m[1][3] * v.y + mat.m[2][3] * v.z + mat.m[3][3]);

		r.x = (mat.m[0][0] * v.x + mat.m[1][0] * v.y + mat.m[2][0] * v.z + mat.m[3][0]) * fInvW;
		r.y = (mat.m[0][1] * v.x + mat.m[1][1] * v.y + mat.m[2][1] * v.z + mat.m[3][1]) * fInvW;
		r.z = (mat.m[0][2] * v.x + mat.m[1][2] * v.y + mat.m[2][2] * v.z + mat.m[3][2]) * fInvW;
		return r;
	}
public:
	static inline Matrix4 makeTrans(float x, float y, float z) {
		return Matrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, z, 1);
	}
	
	static inline Matrix4 makeTrans(const Vector3 pos) {
		return Matrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			pos.x, pos.y, pos.z, 1);
	}

	static inline Matrix4 makeScale(float x, float y, float z) {
		return Matrix4(
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1);
	}

	static inline Matrix4 makeRotate_X(float angle) {
		float s = sinf(angle);
		float c = cosf(angle);
		return Matrix4(
			1, 0, 0, 0,
			0, c, s, 0,
			0, -s, c, 0,
			0, 0, 0, 1);
	}

	static inline Matrix4 makeRotate_Y(float angle) {
		float s = sinf(angle);
		float c = cosf(angle);
		return Matrix4(
			c, 0, s, 0,
			0, 1, 0, 0,
			-s, 0, c, 0,
			0, 0, 0, 1);
	}

	static inline Matrix4 makeRotate_Z(float angle) {
		float s = sinf(angle);
		float c = cosf(angle);
		return Matrix4(
			c, s, 0, 0,
			-s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	inline Matrix4 transpose(void) const {
		return Matrix4(
			m[0][0], m[1][0], m[2][0], m[3][0],
			m[0][1], m[1][1], m[2][1], m[3][1],
			m[0][2], m[1][2], m[2][2], m[3][2],
			m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	//return this^{-1}, return ZERO if no solution
	Matrix4 inverse(void) const;

	//return a left-handed, look-at matrix.
	static Matrix4 lookatLH(const Vector3& eye, const Vector3& lookat, const Vector3& up);
	//return a left-handed perspective projection matrix based on a field of view.
	static Matrix4 perspectiveFovLH(float fov, float aspect, float zNear, float zFar);

public:
	static const Matrix4 ZERO;
	static const Matrix4 IDENTITY;

};

}
