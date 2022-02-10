#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

enum class VertexElementType : size_t {
	VET_POSITION = 0,		
	VET_NORMAL,				
	VET_COLOR,				
	VET_TANGENT,			
	VET_BINORMAL,			
	VET_TEXCOORD0,			
	VET_TEXCOORD1,			
	VET_TEXCOORD2,			
	VET_TEXCOORD3,			
	VET_TEXCOORD4,			
	VET_TEXCOORD5,			
	VET_TEXCOORD6,			
	VET_TEXCOORD7,			

	VET_COUNTS=32,
};

enum VertexElementFormat
{
	VEF_FLOAT_X1,
	VET_FLOAT_X2,
	VET_FLOAT_X3,
	VET_FLOAT_X4,
	VET_UINT32_X1,
	VET_UINT32_X2,
	VET_UINT32_X3,
	VET_UINT32_X4,
};

enum PrimitiveType
{
	PT_POINT_LIST,
	PT_LINE_LIST,
	PT_LINE_STRIP,
	PT_TRIANGLE_LIST,
	PT_TRIANGLE_STRIP,
};

enum PixelFormat
{
	//96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
	PF_FLOAT32_RGB,
	/// 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
	PF_FLOAT32_RGBA,
	// 32-bit pixel format, 32 bits (float) for red
	PF_FLOAT32_R,
	/// 132-bit pixel format, 8 bits (float) for red, 8 bits (float) for green, 8 bits (float) for blue, 8 bits (float) for alpha
	PF_UINT8_RGBA,
};

}
