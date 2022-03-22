#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

typedef int64_t ifloat_t;
typedef int64_t ifloat2_t[2];
typedef int64_t ifloat3_t[3];

#define IFLOAT_SHIFT (16)
#define IFLOAT_SCALE ((double)(1<<IFLOAT_SHIFT))

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_init(float a)
{
	return (ifloat_t)((double)a * IFLOAT_SCALE + 0.5);
}

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_init(int32_t a)
{
	return (ifloat_t)(a<< IFLOAT_SHIFT);
}

//-------------------------------------------------------------------------------------
inline float ifloat_get_float(ifloat_t a)
{
	return (float)(a / IFLOAT_SCALE);
}

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_get_int(ifloat_t a)
{
	return (ifloat_t)(a >> IFLOAT_SHIFT);
}

//-------------------------------------------------------------------------------------
inline void ifloat3_add(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] += b[0]; a[1] += b[1]; a[2] += b[2];
}

//-------------------------------------------------------------------------------------
inline void ifloat3_sub(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2];
}

//-------------------------------------------------------------------------------------
inline void ifloat3_cpy(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] = b[0]; a[1] = b[1]; a[2] = b[2];
}

}
