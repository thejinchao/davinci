#include <davinci.h>
#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "dvt_unit_common.h"

using namespace davinci;

//-------------------------------------------------------------------------------------
TEST(Math_Matrix3, Basic)
{
	{
		float data[3][3] = { { THREE_RANDOM_FLOAT }, { THREE_RANDOM_FLOAT }, { THREE_RANDOM_FLOAT } };

		fMatrix3 m1(data), m2(m1);
		fMatrix3 m3(
			data[0][0], data[0][1], data[0][2],
			data[1][0], data[1][1], data[1][2],
			data[2][0], data[2][1], data[2][2]
		);

		EXPECT_EQ_3X3(m1, data);
		EXPECT_EQ_3X3(m2, data);
		EXPECT_EQ_3X3(m3, data);
	}

	{
		fMatrix3 m1(NINE_RANDOM_FLOAT), m2;

		m2 = m1;
		EXPECT_EQ_3X3(m1, m2);
		EXPECT_TRUE(m1 == m2);

		m2[0][0] += 1.0;
		EXPECT_TRUE(m1 != m2);
	}
}

//-------------------------------------------------------------------------------------
TEST(Math_Matrix3, Algorithm)
{
	//+
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float data2[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1)), m2(NINE_ELEMENT_ROW(data2));
		glm::mat3 gm1(NINE_ELEMENT_COL(data1)), gm2(NINE_ELEMENT_COL(data2));

		fMatrix3 m3 = m1 + m2;
		glm::mat3 gm3 = gm1 + gm2;

		EXPECT_EQ_3X3_T(m3, gm3);
	}

	//-
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float data2[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1)), m2(NINE_ELEMENT_ROW(data2));
		glm::mat3 gm1(NINE_ELEMENT_COL(data1)), gm2(NINE_ELEMENT_COL(data2));

		fMatrix3 m3 = m1 - m2;
		glm::mat3 gm3 = gm1 - gm2;

		EXPECT_EQ_3X3_T(m3, gm3);
	}

	//*
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float data2[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1)), m2(NINE_ELEMENT_ROW(data2));
		glm::mat3 gm1(NINE_ELEMENT_COL(data1)), gm2(NINE_ELEMENT_COL(data2));

		fMatrix3 m3 = m1 * m2;
		glm::mat3 gm3 = gm1 * gm2;

		EXPECT_EQ_3X3_T(m3, gm3);
	}

	// -
	{
		float data1[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1));
		glm::mat3 gm1(NINE_ELEMENT_COL(data1));

		fMatrix3 m2 = -m1;
		glm::mat3 gm2 = -gm1;

		EXPECT_EQ_3X3_T(m2, gm2);
	}


	// vector * matrix
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float data2[3] = { THREE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1));
		fVector3 v1(data2);
		fVector3 v2 = v1 * m1;

		glm::mat3 gm1(NINE_ELEMENT_COL(data1));
		glm::vec3 gv1(data2[0], data2[1], data2[2]);
		glm::vec3 gv2 = gv1*gm1;

		EXPECT_EQ(v2.x, gv2.x);
		EXPECT_EQ(v2.y, gv2.y);
		EXPECT_EQ(v2.z, gv2.z);
	}

	// matrix * vector
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float data2[3] = { THREE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1));
		fVector3 v1(data2);
		fVector3 v2 = m1 * v1;

		glm::mat3 gm1(NINE_ELEMENT_COL(data1));
		glm::vec3 gv1(data2[0], data2[1], data2[2]);
		glm::vec3 gv2 = gm1 * gv1;

		EXPECT_EQ(v2.x, gv2.x);
		EXPECT_EQ(v2.y, gv2.y);
		EXPECT_EQ(v2.z, gv2.z);
	}

	// * float
	{
		float data1[9] = { NINE_RANDOM_FLOAT };
		float a = _randomFloat();

		fMatrix3 m1(NINE_ELEMENT_ROW(data1));
		fMatrix3 m2 = m1 * a, m3 = a * m1;

		glm::mat3 gm1(NINE_ELEMENT_COL(data1));
		glm::mat3 gm2 = gm1 * a;


		EXPECT_EQ_3X3_T(m2, gm2);
		EXPECT_TRUE(m2 == m3);
	}

	// transpose
	{
		float data1[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1));
		fMatrix3 m2 = m1.transpose();

		glm::mat3 gm1(NINE_ELEMENT_COL(data1));
		glm::mat3 gm2 = glm::transpose(gm1);

		EXPECT_EQ_3X3_T(m2, gm2);
	}

	// inverse
	{
		float data1[9] = { NINE_RANDOM_FLOAT };

		fMatrix3 m1(NINE_ELEMENT_ROW(data1)), m2;
		bool inv = m1.inverse(m2);

		glm::mat3 gm1(NINE_ELEMENT_COL(data1));
		glm::mat3 gm2 = glm::inverse(gm1);

		if (inv) 
			EXPECT_EQ_3X3_T(m2, gm2);

		EXPECT_FALSE(fMatrix3::ZERO.inverse(m2));
	}

}

