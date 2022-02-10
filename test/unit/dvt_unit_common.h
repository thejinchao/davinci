#pragma once

#include <davinci.h>
using namespace davinci;

//-------------------------------------------------------------------------------------
inline float _randomFloat(void) {
	return MathUtil::rangeRandom(-65521 / 16.f, 65521 / 16.f);
}

//-------------------------------------------------------------------------------------
inline bool _floatEqualWithRange(float a, float b, float t = std::numeric_limits<float>::epsilon()) {
	if (MathUtil::floatEqual(a, b)) return true;

	float d = std::abs(a - b) / std::min(std::abs(a), std::abs(b));
	return MathUtil::floatEqual(d, 0.f, t);
}

//-------------------------------------------------------------------------------------
#define TWO_RANDOM_FLOAT		_randomFloat(), _randomFloat()
#define THREE_RANDOM_FLOAT		_randomFloat(), _randomFloat(), _randomFloat()
#define FOUR_RANDOM_FLOAT		_randomFloat(), _randomFloat(), _randomFloat(), _randomFloat()

#define NINE_RANDOM_FLOAT		THREE_RANDOM_FLOAT, THREE_RANDOM_FLOAT, THREE_RANDOM_FLOAT
#define SIXTEEN_RANDOM_FLOAT	FOUR_RANDOM_FLOAT, FOUR_RANDOM_FLOAT, FOUR_RANDOM_FLOAT, FOUR_RANDOM_FLOAT

#define THREE_ELEMENT(data)			data[0], data[1], data[2]
#define FOUR_ELEMENT(data)			data[0], data[1], data[2], data[3]
#define NINE_ELEMENT_ROW(data)		data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
#define NINE_ELEMENT_COL(data)		data[0], data[3], data[6], data[1], data[4], data[7], data[2], data[5], data[8]
#define SIXTEEN_ELEMENT_ROW(data)	data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]
#define SIXTEEN_ELEMENT_COL(data)	data[0], data[4], data[8], data[12], data[1], data[5], data[9], data[13], data[2], data[6], data[10], data[14], data[3], data[7], data[11], data[15]

#define EXPECT_EQ_3X3(a, b)		for(size_t i=0; i<3; i++) for(size_t j=0; j<3; j++) EXPECT_EQ(a[i][j], b[i][j]);
#define EXPECT_EQ_3X3_T(a, b)	for(size_t i=0; i<3; i++) for(size_t j=0; j<3; j++) EXPECT_EQ(a[i][j], b[(int32_t)j][(int32_t)i]);
#define EXPECT_EQ_4X4(a, b)		for(size_t i=0; i<4; i++) for(size_t j=0; j<4; j++) EXPECT_EQ(a[i][j], b[i][j]);
#define EXPECT_EQ_4X4_T(a, b)	for(size_t i=0; i<4; i++) for(size_t j=0; j<4; j++) EXPECT_EQ(a[i][j], b[(int32_t)j][(int32_t)i]);

//-------------------------------------------------------------------------------------
#define SIXTEEN_RANDOM_FLOAT2		THREE_RANDOM_FLOAT, 0.f, THREE_RANDOM_FLOAT, 0.f, THREE_RANDOM_FLOAT, 0.f, THREE_RANDOM_FLOAT, 1.f

#define EXPECT_EQ_4X4_T_APPROX(a, b, t) for(size_t ii=0; ii<4; ii++) for(size_t jj=0; jj<4; jj++) \
	EXPECT_TRUE(_floatEqualWithRange(a[ii][jj], b[(int32_t)jj][(int32_t)ii], t));
