#pragma once

#include "dv_prerequisites.h"
#include "dv_math_util.h"

namespace davinci
{

template<typename T>
class TMatrix3
{
protected:
	/* The matrix entries, indexed by [row][col].
        | m[0][0]  m[0][1]  m[0][2] |
		| m[1][0]  m[1][1]  m[1][2] |
		| m[2][0]  m[2][1]  m[2][2] |
	*/
	union {
		T m[3][3];
		T _m[9];
	};

public:
	// constructor, It does NOT initialize the matrix for efficiency.
	TMatrix3() {}
	inline explicit TMatrix3(const T arr[3][3]) {
		memcpy(m, arr, 9 * sizeof(T));
	}
	inline TMatrix3(const TMatrix3& rkMatrix) {
		memcpy(m, rkMatrix.m, 9 * sizeof(T));
	}
	TMatrix3(
		T m00, T m01, T m02,
		T m10, T m11, T m12,
		T m20, T m21, T m22)
	{
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
	}

public:
	inline const T* operator[] (size_t iRow) const {
		assert(iRow < 3);
		return m[iRow];
	}

	inline T* operator[] (size_t iRow) {
		assert(iRow < 3);
		return m[iRow];
	}
public:
	// Operations
	inline TMatrix3& operator = (const TMatrix3& rkMatrix) {
		memcpy(m, rkMatrix.m, 9 * sizeof(T));
		return *this;
	}

	bool operator == (const TMatrix3& rkMatrix) const;
	inline bool operator != (const TMatrix3& rkMatrix) const {
		return !operator==(rkMatrix);
	}

	// return  this + rkMatrix
	TMatrix3 operator + (const TMatrix3& rkMatrix) const;

	// return this - Matrix
	TMatrix3 operator - (const TMatrix3& rkMatrix) const;

	// return this * rkMatrix
	TMatrix3 operator * (const TMatrix3& rkMatrix) const;

	// return -this
	TMatrix3 operator - () const;

	/*  Matrix * Vector
         | m[0][0]  m[0][1]  m[0][2] |   |x|
         | m[1][0]  m[1][1]  m[1][2] | * |y|
         | m[2][0]  m[2][1]  m[2][2] |   |z|
	*/
	TVector3<T> operator * (const TVector3<T>& rkVector) const;

	/* Vector * Matrix
                | m[0][0]  m[0][1]  m[0][2] |
    [x, y, z] * | m[1][0]  m[1][1]  m[1][2] |
                | m[2][0]  m[2][1]  m[2][2] |
	*/
	template<typename U>
	friend TVector3<U> operator * (const TVector3<U>& rkVector, const TMatrix3<U>& rkMatrix);

	/// Matrix * scalar
	TMatrix3 operator * (T fScalar) const;

	/// Scalar * matrix
	template<typename U> 
	friend TMatrix3<U> operator * (U fScalar, const TMatrix3<U>& rkMatrix);

public:
	// Utilities

	//return this^T
	TMatrix3 transpose(void) const;
	//rkInverse = this^{-1}?? return false mean no solution
	bool inverse(TMatrix3& rkInverse, T fTolerance = 1e-06f) const;
	//return this^{-1}, return ZERO if no solution
	TMatrix3 inverse(T fTolerance = 1e-06f) const;

public:
	static const TMatrix3 ZERO;
	static const TMatrix3 IDENTITY;
};

//-------------------------------------------------------------------------------------
template<typename T>
bool TMatrix3<T>::operator== (const TMatrix3<T>& rkMatrix) const
{
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			if (m[iRow][iCol] != rkMatrix.m[iRow][iCol])
				return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::operator + (const TMatrix3<T>& rkMatrix) const
{
	TMatrix3<T> kSum;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			kSum.m[iRow][iCol] = m[iRow][iCol] + rkMatrix.m[iRow][iCol];
		}
	}
	return kSum;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::operator - (const TMatrix3<T>& rkMatrix) const
{
	TMatrix3<T> kDiff;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			kDiff.m[iRow][iCol] = m[iRow][iCol] - rkMatrix.m[iRow][iCol];
		}
	}
	return kDiff;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::operator * (const TMatrix3<T>& rkMatrix) const
{
	TMatrix3<T> kProd;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			kProd.m[iRow][iCol] =
				m[iRow][0] * rkMatrix.m[0][iCol] +
				m[iRow][1] * rkMatrix.m[1][iCol] +
				m[iRow][2] * rkMatrix.m[2][iCol];
		}
	}
	return kProd;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::operator - () const
{
	TMatrix3<T> kNeg;
	for (size_t i = 0; i < 9; i++) {
		kNeg._m[i] = -_m[i];
	}
	return kNeg;
}

//-------------------------------------------------------------------------------------
template<typename T>
TVector3<T> TMatrix3<T>::operator* (const TVector3<T>& rkPoint) const
{
	TVector3<T> kProd(
		m[0][0] * rkPoint[0] + m[0][1] * rkPoint[1] + m[0][2] * rkPoint[2],
		m[1][0] * rkPoint[0] + m[1][1] * rkPoint[1] + m[1][2] * rkPoint[2],
		m[2][0] * rkPoint[0] + m[2][1] * rkPoint[1] + m[2][2] * rkPoint[2]);

	return kProd;
}

//-------------------------------------------------------------------------------------
template<typename T>
inline TVector3<T> operator* (const TVector3<T>& rkPoint, const TMatrix3<T>& rkMatrix)
{
	TVector3<T> kProd(
		rkPoint[0] * rkMatrix.m[0][0] + rkPoint[1] * rkMatrix.m[1][0] + rkPoint[2] * rkMatrix.m[2][0],
		rkPoint[0] * rkMatrix.m[0][1] + rkPoint[1] * rkMatrix.m[1][1] + rkPoint[2] * rkMatrix.m[2][1],
		rkPoint[0] * rkMatrix.m[0][2] + rkPoint[1] * rkMatrix.m[1][2] + rkPoint[2] * rkMatrix.m[2][2]);

	return kProd;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::operator* (T fScalar) const
{
	TMatrix3<T> kProd;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kProd[iRow][iCol] = fScalar * m[iRow][iCol];
	}
	return kProd;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> operator* (T fScalar, const TMatrix3<T>& rkMatrix)
{
	TMatrix3<T> kProd;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kProd[iRow][iCol] = fScalar * rkMatrix.m[iRow][iCol];
	}
	return kProd;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::transpose(void) const
{
	TMatrix3<T> kTranspose;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kTranspose[iRow][iCol] = m[iCol][iRow];
	}
	return kTranspose;
}

//-------------------------------------------------------------------------------------
template<typename T>
bool TMatrix3<T>::inverse(TMatrix3<T>& rkInverse, T fTolerance) const
{
	// Invert a 3x3 using cofactors.  This is about 8 times faster than
	// the Numerical Recipes code which uses Gaussian elimination.

	rkInverse[0][0] = m[1][1] * m[2][2] -
		m[1][2] * m[2][1];
	rkInverse[0][1] = m[0][2] * m[2][1] -
		m[0][1] * m[2][2];
	rkInverse[0][2] = m[0][1] * m[1][2] -
		m[0][2] * m[1][1];
	rkInverse[1][0] = m[1][2] * m[2][0] -
		m[1][0] * m[2][2];
	rkInverse[1][1] = m[0][0] * m[2][2] -
		m[0][2] * m[2][0];
	rkInverse[1][2] = m[0][2] * m[1][0] -
		m[0][0] * m[1][2];
	rkInverse[2][0] = m[1][0] * m[2][1] -
		m[1][1] * m[2][0];
	rkInverse[2][1] = m[0][1] * m[2][0] -
		m[0][0] * m[2][1];
	rkInverse[2][2] = m[0][0] * m[1][1] -
		m[0][1] * m[1][0];

	T fDet =
		m[0][0] * rkInverse[0][0] +
		m[0][1] * rkInverse[1][0] +
		m[0][2] * rkInverse[2][0];

	if (std::abs(fDet) <= fTolerance)
		return false;

	T fInvDet = T(1.0) / fDet;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			rkInverse[iRow][iCol] *= fInvDet;
	}

	return true;
}

//-------------------------------------------------------------------------------------
template<typename T>
TMatrix3<T> TMatrix3<T>::inverse(T fTolerance) const
{
	TMatrix3<T> kInverse = TMatrix3<T>::ZERO;
	inverse(kInverse, fTolerance);
	return kInverse;
}

typedef TMatrix3<float> fMatrix3;

}
