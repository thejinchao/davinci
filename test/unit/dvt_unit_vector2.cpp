#include <davinci.h>
#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "dvt_unit_common.h"

using namespace davinci;

//-------------------------------------------------------------------------------------
TEST(Math_Vector2, Basic)
{
	{
		Vector2 v1;		
		EXPECT_EQ(v1.x, 0.f);
		EXPECT_EQ(v1.y, 0.f);
	}
	{
		float a = _randomFloat(), b = _randomFloat();
		Vector2 v1(a, b), v2(v1);
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);

		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, b);
	}
	{
		float a = _randomFloat(), b = _randomFloat();
		Vector2 v1;
		v1[0] = a; v1[1] = b;

		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, b);

		EXPECT_EQ(v1[0], a);
		EXPECT_EQ(v1[1], b);
	}
	{
		float a = _randomFloat();
		Vector2 v2(a);
		EXPECT_EQ(v2.x, a);
		EXPECT_EQ(v2.y, a);
	}
	{
		float a[2] = { TWO_RANDOM_FLOAT };
		Vector2 v2(a);
		EXPECT_EQ(v2.x, a[0]);
		EXPECT_EQ(v2.y, a[1]);
	}
	{
		int a[2] = { rand() - RAND_MAX/2, rand() - RAND_MAX / 2 };
		Vector2 v2(a);
		EXPECT_EQ(v2.x, (float)a[0]);
		EXPECT_EQ(v2.y, (float)a[1]);
	}

	//operator 
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2;
		v2 = v1;
		EXPECT_EQ(v1.x, v2.x);
		EXPECT_EQ(v1.y, v2.y);
	}
	{
		float a = _randomFloat();
		Vector2 v1;
		v1 = a;
		EXPECT_EQ(v1.x, a);
		EXPECT_EQ(v1.y, a);
	}
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2(v1);
		EXPECT_TRUE(v1 == v2);
		v2.x += 1.f;
		EXPECT_TRUE(v1 != v2);
	}

	{
		float a = 0.f;
		while (a == 0.f) { a = _randomFloat(); }

		Vector2 v1(TWO_RANDOM_FLOAT);

		Vector2 v2;// (_randomFloat(), _randomFloat());
		while (v2.x == 0.f || v2.y == 0.f) {
			v2 = Vector2(TWO_RANDOM_FLOAT);
		}
		Vector2 v3;

		v3 = v1 + v2;
		EXPECT_EQ(v3.x, v1.x + v2.x);
		EXPECT_EQ(v3.y, v1.y + v2.y);
		v3 = v1 - v2;
		EXPECT_EQ(v3.x, v1.x - v2.x);
		EXPECT_EQ(v3.y, v1.y - v2.y);
		v3 = v1 * a;
		EXPECT_EQ(v3.x, v1.x * a);
		EXPECT_EQ(v3.y, v1.y * a);
		v3 = v1 * v2;
		EXPECT_EQ(v3.x, v1.x * v2.x);
		EXPECT_EQ(v3.y, v1.y * v2.y);
		v3 = v1 / a;
		EXPECT_EQ(v3.x, v1.x / a);
		EXPECT_EQ(v3.y, v1.y / a);
		v3 = v1 / v2;
		EXPECT_EQ(v3.x, v1.x / v2.x);
		EXPECT_EQ(v3.y, v1.y / v2.y);
	}

	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2;
		v2 = +v1;
		EXPECT_TRUE(v1 == v2);

		v2 = -v1;
		EXPECT_EQ(v1.x, -v2.x);
		EXPECT_EQ(v1.y, -v2.y);
	}

	{
		float a = _randomFloat();
		Vector2 v1, v2;
		while (v1.x == 0.f || v1.y == 0.f) {
			v1 = Vector2(TWO_RANDOM_FLOAT);
		}

		v2 = a * v1;
		EXPECT_EQ(v2.x, a*v1.x);
		EXPECT_EQ(v2.y, a*v1.y);

		v2 = a / v1;
		EXPECT_EQ(v2.x, a/v1.x);
		EXPECT_EQ(v2.y, a/v1.y);

		v2 = v1 + a;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);

		v2 = v1 - a;
		EXPECT_EQ(v2.x, v1.x - a);
		EXPECT_EQ(v2.y, v1.y - a);

		v2 = a + v1;
		EXPECT_EQ(v2.x, a + v1.x);
		EXPECT_EQ(v2.y, a + v1.y);
	}

	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2, v3;
		while (v2.x == 0.f || v2.y == 0.f) {
			v2 = Vector2(TWO_RANDOM_FLOAT);
		}

		float a = _randomFloat();
		while (a == 0.f) { a = _randomFloat(); }

		v3 = v1;
		v3 += v2;
		EXPECT_TRUE(v3 == v1+v2);

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
		Vector2 v1(TWO_RANDOM_FLOAT), v2(v1);
		
		v2 += 1.f;
		EXPECT_TRUE(v2 > v1);

		v2 -= 2.f;
		EXPECT_TRUE(v2 < v1);
	}
}

//-------------------------------------------------------------------------------------
TEST(Math_Vector2, Algorithm)
{
	//length
	{
		Vector2 v1(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y);
		
		EXPECT_EQ(v1.length(), glm::length(gv1));
	}

	//squaredLength
	{
		Vector2 v1(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y);

		EXPECT_EQ(v1.squaredLength(), glm::dot(gv1, gv1));
	}

	//distance
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y), gv2(v2.x, v2.y);
		
		EXPECT_EQ(v1.distance(v2), glm::distance(gv1, gv2));
	}

	//squaredDistance
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y), gv2(v2.x, v2.y);
		glm::vec2 dis = gv1 - gv2;

		EXPECT_EQ(v1.squaredDistance(v2), glm::dot(dis, dis));
	}

	//dotProduct
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y), gv2(v2.x, v2.y);

		EXPECT_EQ(v1.dotProduct(v2), glm::dot(gv1, gv2));
	}

	//crossProduct
	{
		Vector2 v1(TWO_RANDOM_FLOAT), v2(TWO_RANDOM_FLOAT);
		glm::vec3 gv1(v1.x, v1.y, 0), gv2(v2.x, v2.y, 0);

		EXPECT_EQ(v1.crossProduct(v2), glm::cross(gv1, gv2).z);
	}

	//normalise
	{
		Vector2 v1(TWO_RANDOM_FLOAT);
		glm::vec2 gv1(v1.x, v1.y);

		v1.normalise();
		glm::vec2 gv1_normal = glm::normalize(gv1);

		EXPECT_EQ(v1.x, gv1_normal.x);
		EXPECT_EQ(v1.y, gv1_normal.y);
	}
}
