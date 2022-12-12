#include <davinci.h>
#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "dvt_unit_common.h"

using namespace davinci;

//-------------------------------------------------------------------------------------
TEST(Math_Vector4, Basic)
{
	{
		fVector4 v1;
		EXPECT_EQ(v1.x, 0.f);
		EXPECT_EQ(v1.y, 0.f);
		EXPECT_EQ(v1.z, 0.f);
		EXPECT_EQ(v1.w, 0.f);
	}
	{
		float a = _randomFloat(), b = _randomFloat(), c = _randomFloat(), d = _randomFloat();
		fVector4 v1(a, b, c, d), v2(v1);
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);
		EXPECT_EQ(v1.z, c);
		EXPECT_EQ(v1.w, d);

		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, b);
		EXPECT_EQ(v2.z, c);
		EXPECT_EQ(v2.w, d);
	}
	{
		float a = _randomFloat(), b = _randomFloat(), c = _randomFloat(), d = _randomFloat();
		fVector4 v1;
		v1[0] = a; v1[1] = b; v1[2] = c; v1[3] = d;

		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);
		EXPECT_EQ(v1.z, c);
		EXPECT_EQ(v1.w, d);

		EXPECT_EQ(v1[0], a);
		EXPECT_EQ(v1[1], b);
		EXPECT_EQ(v1[2], c);
		EXPECT_EQ(v1[3], d);
	}
	{
		float a = _randomFloat();
		fVector4 v2(a);
		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, a);
		EXPECT_EQ(v2.z, a);
		EXPECT_EQ(v2.w, a);
	}
	{
		float a[4] = { FOUR_RANDOM_FLOAT };
		fVector4 v2(a);
		EXPECT_EQ(v2.x, a[0]);
		EXPECT_EQ(v2.y, a[1]);
		EXPECT_EQ(v2.z, a[2]);
		EXPECT_EQ(v2.w, a[3]);
	}
	{
		int a[4] = { rand() - RAND_MAX / 2, rand() - RAND_MAX / 2,  rand() - RAND_MAX / 2, rand() - RAND_MAX / 2 };
		fVector4 v2(a);
		EXPECT_EQ(v2.x, (float)a[0]);
		EXPECT_EQ(v2.y, (float)a[1]);
		EXPECT_EQ(v2.z, (float)a[2]);
		EXPECT_EQ(v2.w, (float)a[3]);
	}

	//operator 
	{
		fVector4 v1(FOUR_RANDOM_FLOAT), v2;
		v2 = v1;
		EXPECT_EQ(v1.x, v2.x);
		EXPECT_EQ(v1.y, v2.y);
		EXPECT_EQ(v1.z, v2.z);
		EXPECT_EQ(v1.w, v2.w);
	}
	{
		float a = _randomFloat();
		fVector4 v1;
		v1 = a;
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, a);
		EXPECT_EQ(v1.z, a);
		EXPECT_EQ(v1.w, a);
	}
	{
		fVector4 v1(FOUR_RANDOM_FLOAT), v2(v1);
		EXPECT_TRUE(v1 == v2);
		v2.x += 1.f;
		EXPECT_TRUE(v1 != v2);
	}
	{
		float a = 0.f;
		while (a == 0.f) { a = _randomFloat(); }

		fVector4 v1(FOUR_RANDOM_FLOAT);

		fVector4 v2;
		while (v2.x == 0.f || v2.y == 0.f || v2.z == 0.f || v2.w == 0.f) {
			v2 = fVector4(FOUR_RANDOM_FLOAT);
		}
		fVector4 v3;

		v3 = v1 + v2;
		EXPECT_EQ(v3.x, v1.x + v2.x);
		EXPECT_EQ(v3.y, v1.y + v2.y);
		EXPECT_EQ(v3.z, v1.z + v2.z);
		EXPECT_EQ(v3.w, v1.w + v2.w);
		v3 = v1 - v2;
		EXPECT_EQ(v3.x, v1.x - v2.x);
		EXPECT_EQ(v3.y, v1.y - v2.y);
		EXPECT_EQ(v3.z, v1.z - v2.z);
		EXPECT_EQ(v3.w, v1.w - v2.w);
		v3 = v1 * a;
		EXPECT_EQ(v3.x, v1.x * a);
		EXPECT_EQ(v3.y, v1.y * a);
		EXPECT_EQ(v3.z, v1.z * a);
		EXPECT_EQ(v3.w, v1.w * a);
		v3 = v1 * v2;
		EXPECT_EQ(v3.x, v1.x * v2.x);
		EXPECT_EQ(v3.y, v1.y * v2.y);
		EXPECT_EQ(v3.z, v1.z * v2.z);
		EXPECT_EQ(v3.w, v1.w * v2.w);
		v3 = v1 / a;
		EXPECT_EQ(v3.x, v1.x / a);
		EXPECT_EQ(v3.y, v1.y / a);
		EXPECT_EQ(v3.z, v1.z / a);
		EXPECT_EQ(v3.w, v1.w / a);
		v3 = v1 / v2;
		EXPECT_EQ(v3.x, v1.x / v2.x);
		EXPECT_EQ(v3.y, v1.y / v2.y);
		EXPECT_EQ(v3.z, v1.z / v2.z);
		EXPECT_EQ(v3.w, v1.w / v2.w);
	}
	{
		fVector4 v1(FOUR_RANDOM_FLOAT), v2;
		v2 = +v1;
		EXPECT_TRUE(v1 == v2);

		v2 = -v1;
		EXPECT_EQ(v1.x, -v2.x);
		EXPECT_EQ(v1.y, -v2.y);
		EXPECT_EQ(v1.z, -v2.z);
		EXPECT_EQ(v1.w, -v2.w);
	}

	{
		float a = _randomFloat();
		fVector4 v1, v2;
		while (v1.x == 0.f || v1.y == 0.f || v1.z == 0.f || v1.w == 0.f) {
			v1 = fVector4(FOUR_RANDOM_FLOAT);
		}

		v2 = a * v1;
		EXPECT_EQ(v2.x, a*v1.x);
		EXPECT_EQ(v2.y, a*v1.y);
		EXPECT_EQ(v2.z, a*v1.z);
		EXPECT_EQ(v2.w, a*v1.w);

		v2 = a / v1;
		EXPECT_EQ(v2.x, a / v1.x);
		EXPECT_EQ(v2.y, a / v1.y);
		EXPECT_EQ(v2.z, a / v1.z);
		EXPECT_EQ(v2.w, a / v1.w);

		v2 = v1 + a;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);
		EXPECT_EQ(v2.w, a + v1.w);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);
		EXPECT_EQ(v2.w, a + v1.w);

		v2 = v1 - a;
		EXPECT_EQ(v2.x, v1.x - a);
		EXPECT_EQ(v2.y, v1.y - a);
		EXPECT_EQ(v2.z, v1.z - a);
		EXPECT_EQ(v2.w, v1.w - a);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
		EXPECT_EQ(v2.z, a + v1.z);
		EXPECT_EQ(v2.w, a + v1.w);
	}

	{
		fVector4 v1(FOUR_RANDOM_FLOAT), v2, v3;
		while (v2.x == 0.f || v2.y == 0.f || v2.z == 0.f || v2.w == 0.f) {
			v2 = fVector4(FOUR_RANDOM_FLOAT);
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
		fVector4 v1(FOUR_RANDOM_FLOAT), v2(v1);

		v2 += 1.f;
		EXPECT_TRUE(v2 > v1);

		v2 -= 2.f;
		EXPECT_TRUE(v2 < v1);
	}
}


//-------------------------------------------------------------------------------------
TEST(Math_Vector4, Algorithm)
{

	//dotProduct
	{
		fVector4 v1(FOUR_RANDOM_FLOAT), v2(FOUR_RANDOM_FLOAT);
		glm::vec4 gv1(v1.x, v1.y, v1.z, v1.w), gv2(v2.x, v2.y, v2.z, v2.w);

		EXPECT_EQ(v1.dotProduct(v2), glm::dot(gv1, gv2));
	}

}
