#include <davinci.h>
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "dvt_unit_common.h"

using namespace davinci;

//-------------------------------------------------------------------------------------
TEST(Math_Matrix4, Basic)
{
	{
		float data[4][4] = { { FOUR_RANDOM_FLOAT },{ FOUR_RANDOM_FLOAT },{ FOUR_RANDOM_FLOAT }, { FOUR_RANDOM_FLOAT } };

		fMatrix4 m1(data), m2(m1);
		fMatrix4 m3(
			data[0][0], data[0][1], data[0][2], data[0][3],
			data[1][0], data[1][1], data[1][2], data[1][3],
			data[2][0], data[2][1], data[2][2], data[2][3],
			data[3][0], data[3][1], data[3][2], data[3][3]
			);

		EXPECT_EQ_4X4(m1, data);
		EXPECT_EQ_4X4(m2, data);
		EXPECT_EQ_4X4(m3, data);
	}

	{
		fMatrix4 m1(SIXTEEN_RANDOM_FLOAT), m2;

		m2 = m1;
		EXPECT_EQ_4X4(m1, m2);
		EXPECT_TRUE(m1 == m2);

		m2[0][0] += 1.0;
		EXPECT_TRUE(m1 != m2);
	}
}

//-------------------------------------------------------------------------------------
TEST(Math_Matrix4, Algorithm)
{
	//+
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1)), m2(SIXTEEN_ELEMENT_ROW(data2));
		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1)), gm2(SIXTEEN_ELEMENT_COL(data2));

		fMatrix4 m3 = m1 + m2;
		glm::mat4 gm3 = gm1 + gm2;

		EXPECT_EQ_4X4_T(m3, gm3);
	}

	//-
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1)), m2(SIXTEEN_ELEMENT_ROW(data2));
		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1)), gm2(SIXTEEN_ELEMENT_COL(data2));

		fMatrix4 m3 = m1 - m2;
		glm::mat4 gm3 = gm1 - gm2;

		EXPECT_EQ_4X4_T(m3, gm3);
	}

	//*
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1)), m2(SIXTEEN_ELEMENT_ROW(data2));
		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1)), gm2(SIXTEEN_ELEMENT_COL(data2));

		fMatrix4 m3 = m1 * m2;
		glm::mat4 gm3 = gm1 * gm2;

		EXPECT_EQ_4X4_T(m3, gm3);
	}

	// -
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1));
		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1));

		fMatrix4 m2 = -m1;
		glm::mat4 gm2 = -gm1;

		EXPECT_EQ_4X4_T(m2, gm2);
	}

	// vector4 * matrix
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[4] = { FOUR_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1));
		fVector4 v1(data2);
		fVector4 v2 = v1 * m1;

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1));
		glm::vec4 gv1(FOUR_ELEMENT(data2));
		glm::vec4 gv2 = gv1 * gm1;

		//fVector4 v2 = v1 * m1;
		EXPECT_TRUE(_floatEqualWithRange(v2.x, gv2.x, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.y, gv2.y, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.z, gv2.z, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.w, gv2.w, std::numeric_limits<float>::epsilon() * 100));
	}

	//  matrix * vector4
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[4] = { FOUR_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1));
		fVector4 v1(data2);
		fVector4 v2 = m1 * v1;

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1));
		glm::vec4 gv1(FOUR_ELEMENT(data2));
		glm::vec4 gv2 = gm1 * gv1;

		//fVector4 v2 = v1 * m1;
		EXPECT_TRUE(_floatEqualWithRange(v2.x, gv2.x, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.y, gv2.y, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.z, gv2.z, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.w, gv2.w, std::numeric_limits<float>::epsilon() * 100));
	}

	//  matrix * vector3
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[3] = { THREE_RANDOM_FLOAT};

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1));
		fVector3 v1(data2);
		fVector3 v2 = m1 * v1;

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1));
		glm::vec4 gv1(THREE_ELEMENT(data2), 1);
		glm::vec4 gv2 = gm1 * gv1;
		gv2 /= gv2.w;

		//fVector4 v2 = v1 * m1;
		EXPECT_TRUE(_floatEqualWithRange(v2.x, gv2.x, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.y, gv2.y, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.z, gv2.z, std::numeric_limits<float>::epsilon() * 100));
	}

	// vector3 * matrix
	{
		float data1[16] = { SIXTEEN_RANDOM_FLOAT };
		float data2[3] = { THREE_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data1));
		fVector3 v1(data2);
		fVector3 v2 = v1 * m1;

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data1));
		glm::vec4 gv1(THREE_ELEMENT(data2), 1);
		glm::vec4 gv2 = gv1 * gm1;
		gv2 /= gv2.w;

		EXPECT_TRUE(_floatEqualWithRange(v2.x, gv2.x, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.y, gv2.y, std::numeric_limits<float>::epsilon() * 100));
		EXPECT_TRUE(_floatEqualWithRange(v2.z, gv2.z, std::numeric_limits<float>::epsilon() * 100));
	}

	// * float
	{
		float data[16] = { SIXTEEN_RANDOM_FLOAT };
		float a = _randomFloat();

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data));
		fMatrix4 m2 = m1 * a, m3 = a * m1;

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data));
		glm::mat4 gm2 = gm1 * a;

		EXPECT_EQ_4X4_T(m2, gm2);
		EXPECT_TRUE(m2 == m3);
	}

	// translation
	{
		float trans_data[] = { THREE_RANDOM_FLOAT };

		fMatrix4 m1 = fMatrix4::makeTrans(THREE_ELEMENT(trans_data));
		glm::mat4 gm1 = glm::translate(glm::mat4(1), glm::vec3(THREE_ELEMENT(trans_data)));

		glm::mat4 gm1_t = glm::transpose(gm1);
		EXPECT_EQ_4X4_T(m1, gm1_t);
	}

	// scale
	{
		float scale_data[] = { THREE_RANDOM_FLOAT };

		fMatrix4 m1 = fMatrix4::makeScale(THREE_ELEMENT(scale_data));
		glm::mat4 gm1 = glm::scale(glm::mat4(1.0f), glm::vec3(THREE_ELEMENT(scale_data)));

		EXPECT_EQ_4X4_T(m1, gm1);
	}

	// transpose
	{
		float data[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data));
		fMatrix4 m2 = m1.transpose();

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data));
		glm::mat4 gm2 = glm::transpose(gm1);

		EXPECT_EQ_4X4_T(m2, gm2);
	}

	// inverse
	{
		float data[16] = { SIXTEEN_RANDOM_FLOAT };

		fMatrix4 m1(SIXTEEN_ELEMENT_ROW(data));
		fMatrix4 m2 = m1.inverse();

		glm::mat4 gm1(SIXTEEN_ELEMENT_COL(data));
		glm::mat4 gm2 = glm::inverse(gm1);

		EXPECT_EQ_4X4_T_APPROX(m2, gm2, std::numeric_limits<float>::epsilon()*1000);
	}

	//lookat view matrix
	{
		float dEye[3] = { 0.0f, 4.0f, -10.0f };
		float dAt[3]  = { 0.0f, 1.0f, 0.0f };
		float dUp[3]  = { 0.0f, 1.0f, 0.0f };

		fMatrix4 m1 = fMatrix4::lookatLH(
			fVector3(THREE_ELEMENT(dEye)),
			fVector3(THREE_ELEMENT(dAt)),
			fVector3(THREE_ELEMENT(dUp)));

		glm::mat4 gm1 = glm::lookAtLH(glm::vec3(THREE_ELEMENT(dEye)), glm::vec3(THREE_ELEMENT(dAt)), glm::vec3(THREE_ELEMENT(dUp)));
		glm::mat4 gm1_t = glm::transpose(gm1);

		EXPECT_EQ_4X4_T(m1, gm1_t);
	}

	//lookat view matrix(random)
	{
		for (int i = 0; i < 10; i++) {
			fVector3 dEye, dAt, dUp;
			do {
				dEye[0] = _randomFloat(); dEye[1] = _randomFloat(); dEye[2] = _randomFloat();
				dAt[0] = _randomFloat(); dAt[1] = _randomFloat(); dAt[2] = _randomFloat();
				dUp[0] = _randomFloat(); dUp[1] = _randomFloat(); dUp[2] = _randomFloat();

			} while ((dEye==dAt) || (dUp == fVector3::ZERO));

			fMatrix4 m1 = fMatrix4::lookatLH(
				fVector3(THREE_ELEMENT(dEye)),
				fVector3(THREE_ELEMENT(dAt)),
				fVector3(THREE_ELEMENT(dUp)));

			glm::mat4 gm1 = glm::lookAtLH(glm::vec3(THREE_ELEMENT(dEye)), glm::vec3(THREE_ELEMENT(dAt)), glm::vec3(THREE_ELEMENT(dUp)));
			glm::mat4 gm1_t = glm::transpose(gm1);

			EXPECT_EQ_4X4_T_APPROX(m1, gm1_t, std::numeric_limits<float>::epsilon()*1000);
		}
	}

	//perspective
	{
		float width = 640.f, height = 480.f;

		fMatrix4 m1 = fMatrix4::perspectiveFovLH(MathUtil::PI_DIV4, width / height, 0.01f, 100.f);
		glm::mat4 gm1 = glm::perspectiveLH(MathUtil::PI_DIV4, width / height, 0.01f, 100.f);
		glm::mat4 gm1_t = glm::transpose(gm1);

		EXPECT_EQ_4X4_T_APPROX(m1, gm1_t, std::numeric_limits<float>::epsilon());
	}

	//perspective(random)
	{
		for (int i = 0; i < 10; i++) {
			float fov = MathUtil::rangeRandom(0.1f, MathUtil::PI);
			float aspect = MathUtil::rangeRandom(0.1f, 100.f);
			float zNear = MathUtil::rangeRandom(0.00001f, 0.01f);
			float zFar = MathUtil::rangeRandom(100.f, 10000.f);

			fMatrix4 m1 = fMatrix4::perspectiveFovLH(fov, aspect, zNear, zFar);
			glm::mat4 gm1 = glm::perspectiveLH(fov, aspect, zNear, zFar);
			glm::mat4 gm1_t = glm::transpose(gm1);

			EXPECT_EQ_4X4_T_APPROX(m1, gm1_t, std::numeric_limits<float>::epsilon()*100);
		}
	}
}
