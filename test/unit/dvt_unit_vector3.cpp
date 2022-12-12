#include <davinci.h>
#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "dvt_unit_common.h"

using namespace davinci;

//-------------------------------------------------------------------------------------
TEST(Math_Vector3, Basic)
{
	{
		fVector3 v1;
		EXPECT_EQ(v1.x, 0.f);
		EXPECT_EQ(v1.y, 0.f);
		EXPECT_EQ(v1.z, 0.f);
	}
	{
		float a = _randomFloat(), b = _randomFloat(), c = _randomFloat();
		fVector3 v1(a, b, c), v2(v1);
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);
		EXPECT_EQ(v1.z, c);

		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, b);
		EXPECT_EQ(v2.z, c);
	}
	{
		float a = _randomFloat(), b = _randomFloat(), c = _randomFloat();
		fVector3 v1;
		v1[0] = a; v1[1] = b; v1[2] = c;

		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);
		EXPECT_EQ(v1.z, c);

		EXPECT_EQ(v1[0], a);
		EXPECT_EQ(v1[1], b);
		EXPECT_EQ(v1[2], c);

		fVector2 v1_xy = v1.xy();
		EXPECT_EQ(v1_xy.x, a);
		EXPECT_EQ(v1_xy.y, b);

		fVector2 v1_xz = v1.xz();
		EXPECT_EQ(v1_xz.x, a);
		EXPECT_EQ(v1_xz.y, c);

		fVector2 v1_yz = v1.yz();
		EXPECT_EQ(v1_yz.x, b);
		EXPECT_EQ(v1_yz.y, c);
	}
	{
		float a = _randomFloat();
		fVector3 v2(a);
		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, a);
		EXPECT_EQ(v2.z, a);
	}
	{
		float a[3] = { THREE_RANDOM_FLOAT };
		fVector3 v2(a);
		EXPECT_EQ(v2.x, a[0]);
		EXPECT_EQ(v2.y, a[1]);
		EXPECT_EQ(v2.z, a[2]);
	}
	{
		int a[3] = { rand() - RAND_MAX / 2, rand() - RAND_MAX / 2,  rand() - RAND_MAX / 2 };
		fVector3 v2(a);
		EXPECT_EQ(v2.x, (float)a[0]);
		EXPECT_EQ(v2.y, (float)a[1]);
		EXPECT_EQ(v2.z, (float)a[2]);
	}
	{
		float a = _randomFloat(), b = _randomFloat(), c = _randomFloat();
		fVector2 v1(a, b);
		fVector3 v2(v1, c);
		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, b);
		EXPECT_EQ(v2.z, c);

		fVector2 v3(a, b);
		fVector3 v4(c, v3);
		EXPECT_EQ(v4.x, c);
		EXPECT_EQ(v4.y, a);
		EXPECT_EQ(v4.z, b);
	}

	//operator 
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2;
		v2 = v1;
		EXPECT_EQ(v1.x, v2.x);
		EXPECT_EQ(v1.y, v2.y);
		EXPECT_EQ(v1.z, v2.z);
	}
	{
		float a = _randomFloat();
		fVector3 v1;
		v1 = a;
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, a);
		EXPECT_EQ(v1.z, a);
	}
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(v1);
		EXPECT_TRUE(v1 == v2);
		v2.x += 1.f;
		EXPECT_TRUE(v1 != v2);
	}
	{
		float a = 0.f;
		while (a == 0.f) { a = _randomFloat(); }

		fVector3 v1(THREE_RANDOM_FLOAT);

		fVector3 v2;
		while (v2.x == 0.f || v2.y == 0.f || v2.z == 0.f) {
			v2 = fVector3(THREE_RANDOM_FLOAT);
		}
		fVector3 v3;

		v3 = v1 + v2;
		EXPECT_EQ(v3.x, v1.x + v2.x);
		EXPECT_EQ(v3.y, v1.y + v2.y);
		EXPECT_EQ(v3.z, v1.z + v2.z);
		v3 = v1 - v2;
		EXPECT_EQ(v3.x, v1.x - v2.x);
		EXPECT_EQ(v3.y, v1.y - v2.y);
		EXPECT_EQ(v3.z, v1.z - v2.z);
		v3 = v1 * a;
		EXPECT_EQ(v3.x, v1.x * a);
		EXPECT_EQ(v3.y, v1.y * a);
		EXPECT_EQ(v3.z, v1.z * a);
		v3 = v1 * v2;
		EXPECT_EQ(v3.x, v1.x * v2.x);
		EXPECT_EQ(v3.y, v1.y * v2.y);
		EXPECT_EQ(v3.z, v1.z * v2.z);
		v3 = v1 / a;
		EXPECT_EQ(v3.x, v1.x / a);
		EXPECT_EQ(v3.y, v1.y / a);
		EXPECT_EQ(v3.z, v1.z / a);
		v3 = v1 / v2;
		EXPECT_EQ(v3.x, v1.x / v2.x);
		EXPECT_EQ(v3.y, v1.y / v2.y);
		EXPECT_EQ(v3.z, v1.z / v2.z);
	}
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2;
		v2 = +v1;
		EXPECT_TRUE(v1 == v2);

		v2 = -v1;
		EXPECT_EQ(v1.x, -v2.x);
		EXPECT_EQ(v1.y, -v2.y);
		EXPECT_EQ(v1.z, -v2.z);
	}

	{
		float a = _randomFloat();
		fVector3 v1, v2;
		while (v1.x == 0.f || v1.y == 0.f || v1.z == 0.f) {
			v1 = fVector3(THREE_RANDOM_FLOAT);
		}

		v2 = a * v1;
		EXPECT_EQ(v2.x, a*v1.x);
		EXPECT_EQ(v2.y, a*v1.y);
		EXPECT_EQ(v2.z, a*v1.z);

		v2 = a / v1;
		EXPECT_EQ(v2.x, a / v1.x);
		EXPECT_EQ(v2.y, a / v1.y);
		EXPECT_EQ(v2.z, a / v1.z);

		v2 = v1 + a;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);

		v2 = v1 - a;
		EXPECT_EQ(v2.x, v1.x - a);
		EXPECT_EQ(v2.y, v1.y - a);
		EXPECT_EQ(v2.z, v1.z - a);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);
	}

	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2, v3;
		while (v2.x == 0.f || v2.y == 0.f || v2.z == 0.f) {
			v2 = fVector3(THREE_RANDOM_FLOAT);
		}

		float a = _randomFloat();
		while (a == 0.f) { a = _randomFloat(); }

		v3 = v1;
		v3 += v2;
		EXPECT_TRUE(v3 == v1 + v2);

		v3 = v1;
		v3 += a;
		EXPECT_TRUE(v3 == v1 + a);

		v3 = v1;
		v3 -= v2;
		EXPECT_TRUE(v3 == v1 - v2);

		v3 = v1;
		v3 -= a;
		EXPECT_TRUE(v3 == v1 - a);

		v3 = v1;
		v3 *= v2;
		EXPECT_TRUE(v3 == v1 * v2);

		v3 = v1;
		v3 *= a;
		EXPECT_TRUE(v3 == v1 * a);

		v3 = v1;
		v3 /= v2;
		EXPECT_TRUE(v3 == v1 / v2);

		v3 = v1;
		v3 /= a;
		EXPECT_TRUE(v3 == v1 / a);
	}

	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(v1);

		v2 += 1.f;
		EXPECT_TRUE(v2 > v1);

		v2 -= 2.f;
		EXPECT_TRUE(v2 < v1);
	}
}

//-------------------------------------------------------------------------------------
TEST(Math_Vector3, Algorithm)
{
	//length
	{
		fVector3 v1(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z);
		
		EXPECT_EQ(v1.length(), glm::length(gv1));
	}

	//squaredLength
	{
		fVector3 v1(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z);

		EXPECT_EQ(v1.squaredLength(), glm::dot(gv1, gv1));
	}

	//distance
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z), gv2(v2.x, v2.y, v2.z);
		
		EXPECT_EQ(v1.distance(v2), glm::distance(gv1, gv2));
	}

	//squaredDistance
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z), gv2(v2.x, v2.y, v2.z);
		glm::vec3 len(gv1 - gv2);

		EXPECT_EQ(v1.squaredDistance(v2), glm::dot(len, len));
	}

	//dotProduct
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z), gv2(v2.x, v2.y, v2.z);

		EXPECT_EQ(v1.dotProduct(v2), glm::dot(gv1, gv2));
	}

	//crossProduct
	{
		fVector3 v1(THREE_RANDOM_FLOAT), v2(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z), gv2(v2.x, v2.y, v2.z);

		fVector3 v3 = v1.crossProduct(v2);
		glm::vec3 gv1_cross = glm::cross(gv1, gv2);

		EXPECT_EQ(v3.x, gv1_cross.x);
		EXPECT_EQ(v3.y, gv1_cross.y);
		EXPECT_EQ(v3.z, gv1_cross.z);
	}

	//normalise
	{
		fVector3 v1(THREE_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, v1.z);

		v1.normalise();
		glm::vec3 gv1_nor = glm::normalize(gv1);

		EXPECT_EQ(v1.x, gv1_nor.x);
		EXPECT_EQ(v1.y, gv1_nor.y);
		EXPECT_EQ(v1.z, gv1_nor.z);
	}
}
