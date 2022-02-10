#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

class Matrix3
{
protected:
	/* The matrix entries, indexed by [row][col].
        | m[0][0]  m[0][1]  m[0][2] |
		| m[1][0]  m[1][1]  m[1][2] |
		| m[2][0]  m[2][1]  m[2][2] |
	*/
	union {
		float m[3][3];
		float _m[9];
	};

public:
	// constructor, It does NOT initialize the matrix for efficiency.
	Matrix3() {}
	inline explicit Matrix3(const float arr[3][3]) {
		memcpy(m, arr, 9 * sizeof(float));
	}
	inline Matrix3(const Matrix3& rkMatrix) {
		memcpy(m, rkMatrix.m, 9 * sizeof(float));
	}
	Matrix3(
		float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22)
	{
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
	}

public:
	inline const float* operator[] (size_t iRow) const {
		assert(iRow < 3);
		return m[iRow];
	}

	inline float* operator[] (size_t iRow) {
		assert(iRow < 3);
		return m[iRow];
	}
public:
	// Operations
	inline Matrix3& operator = (const Matrix3& rkMatrix) {
		memcpy(m, rkMatrix.m, 9 * sizeof(float));
		return *this;
	}

	bool operator == (const Matrix3& rkMatrix) const;
	inline bool operator != (const Matrix3& rkMatrix) const {
		return !operator==(rkMatrix);
	}

	// return  this + rkMatrix
	Matrix3 operator + (const Matrix3& rkMatrix) const;

	// return this - Matrix
	Matrix3 operator - (const Matrix3& rkMatrix) const;

	// return this * rkMatrix
	Matrix3 operator * (const Matrix3& rkMatrix) const;

	// return -this
	Matrix3 operator - () const;

	/*  Matrix * Vector
         | m[0][0]  m[0][1]  m[0][2] |   |x|
         | m[1][0]  m[1][1]  m[1][2] | * |y|
         | m[2][0]  m[2][1]  m[2][2] |   |z|
	*/
	Vector3 operator * (const Vector3& rkVector) const;

	/* Vector * Matrix
                | m[0][0]  m[0][1]  m[0][2] |
    [x, y, z] * | m[1][0]  m[1][1]  m[1][2] |
                | m[2][0]  m[2][1]  m[2][2] |
	*/
	friend Vector3 operator * (const Vector3& rkVector, const Matrix3& rkMatrix);

	/// Matrix * scalar
	Matrix3 operator * (float fScalar) const;

	/// Scalar * matrix
	friend Matrix3 operator* (float fScalar, const Matrix3& rkMatrix);

public:
	// Utilities

	//return this^T
	Matrix3 transpose(void) const;
	//rkInverse = this^{-1}?? return false mean no solution
	bool inverse(Matrix3& rkInverse, float fTolerance = 1e-06f) const;
	//return this^{-1}, return ZERO if no solution
	Matrix3 inverse(float fTolerance = 1e-06f) const;

public:
	static const Matrix3 ZERO;
	static const Matrix3 IDENTITY;
};

}
