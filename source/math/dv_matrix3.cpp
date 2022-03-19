#include "dv_precompiled.h"
#include "dv_matrix3.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
const Matrix3 Matrix3::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);
const Matrix3 Matrix3::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);

//-------------------------------------------------------------------------------------
bool Matrix3::operator== (const Matrix3& rkMatrix) const
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
Matrix3 Matrix3::operator + (const Matrix3& rkMatrix) const
{
	Matrix3 kSum;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			kSum.m[iRow][iCol] = m[iRow][iCol] + rkMatrix.m[iRow][iCol];
		}
	}
	return kSum;
}

//-------------------------------------------------------------------------------------
Matrix3 Matrix3::operator - (const Matrix3& rkMatrix) const
{
	Matrix3 kDiff;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++) {
			kDiff.m[iRow][iCol] = m[iRow][iCol] - rkMatrix.m[iRow][iCol];
		}
	}
	return kDiff;
}

//-------------------------------------------------------------------------------------
Matrix3 Matrix3::operator * (const Matrix3& rkMatrix) const
{
	Matrix3 kProd;
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
Matrix3 Matrix3::operator - () const
{
	Matrix3 kNeg;
	for (size_t i = 0; i < 9; i++) {
		kNeg._m[i] = -_m[i];
	}
	return kNeg;
}

//-------------------------------------------------------------------------------------
Vector3 Matrix3::operator* (const Vector3& rkPoint) const
{
	Vector3 kProd(
		m[0][0] * rkPoint[0] + m[0][1] * rkPoint[1] + m[0][2] * rkPoint[2],
		m[1][0] * rkPoint[0] + m[1][1] * rkPoint[1] + m[1][2] * rkPoint[2],
		m[2][0] * rkPoint[0] + m[2][1] * rkPoint[1] + m[2][2] * rkPoint[2]);

	return kProd;
}

//-------------------------------------------------------------------------------------
Vector3 operator* (const Vector3& rkPoint, const Matrix3& rkMatrix)
{
	Vector3 kProd(
		rkPoint[0] * rkMatrix.m[0][0] + rkPoint[1] * rkMatrix.m[1][0] + rkPoint[2] * rkMatrix.m[2][0],
		rkPoint[0] * rkMatrix.m[0][1] + rkPoint[1] * rkMatrix.m[1][1] + rkPoint[2] * rkMatrix.m[2][1],
		rkPoint[0] * rkMatrix.m[0][2] + rkPoint[1] * rkMatrix.m[1][2] + rkPoint[2] * rkMatrix.m[2][2]);

	return kProd;
}

//-------------------------------------------------------------------------------------
Matrix3 Matrix3::operator* (float fScalar) const
{
	Matrix3 kProd;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kProd[iRow][iCol] = fScalar*m[iRow][iCol];
	}
	return kProd;
}

//-------------------------------------------------------------------------------------
Matrix3 operator* (float fScalar, const Matrix3& rkMatrix)
{
	Matrix3 kProd;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kProd[iRow][iCol] = fScalar*rkMatrix.m[iRow][iCol];
	}
	return kProd;
}

//-------------------------------------------------------------------------------------
Matrix3 Matrix3::transpose(void) const
{
	Matrix3 kTranspose;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			kTranspose[iRow][iCol] = m[iCol][iRow];
	}
	return kTranspose;
}

//-------------------------------------------------------------------------------------
bool Matrix3::inverse(Matrix3& rkInverse, float fTolerance) const
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

	float fDet =
		m[0][0] * rkInverse[0][0] +
		m[0][1] * rkInverse[1][0] +
		m[0][2] * rkInverse[2][0];

	if (std::abs(fDet) <= fTolerance)
		return false;

	float fInvDet = 1.0f / fDet;
	for (size_t iRow = 0; iRow < 3; iRow++) {
		for (size_t iCol = 0; iCol < 3; iCol++)
			rkInverse[iRow][iCol] *= fInvDet;
	}

	return true;
}

//-------------------------------------------------------------------------------------
Matrix3 Matrix3::inverse(float fTolerance) const
{
	Matrix3 kInverse = Matrix3::ZERO;
	inverse(kInverse, fTolerance);
	return kInverse;
}

}
