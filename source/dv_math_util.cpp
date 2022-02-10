#include "dv_precompiled.h"
#include "dv_math_util.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
const float MathUtil::POS_INFINITY = std::numeric_limits<float>::infinity();
const float MathUtil::NEG_INFINITY = -std::numeric_limits<float>::infinity();
const float MathUtil::PI = float(4.0 * atan(1.0));
const float MathUtil::PI_X2 = float(2.0 * PI);
const float MathUtil::PI_DIV2 = float(0.5 * PI);
const float MathUtil::PI_DIV4 = float(0.25 * PI);
const float MathUtil::fDeg2Rad = PI / float(180.0);
const float MathUtil::fRad2Deg = float(180.0) / PI;

//-------------------------------------------------------------------------------------
float MathUtil::unitRandom(void)
{
	return float(rand()) / RAND_MAX;
}

}