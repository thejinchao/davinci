#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class AssetUtility
{
public:
	//create standard model
	static ModelPtr createStandardModel_Box(RenderDevice* device, float x_size, float y_size, float z_size, PrimitiveType primitiveType, bool color, bool uv, bool normal);
	static ModelPtr createStandardModel_Sphere(RenderDevice* device, float radius, PrimitiveType primitiveType, int32_t widthSegments = 8, int32_t heightSegments = 6);

	//load model from json file
	static ModelPtr loadModel(RenderDevice* device, const char* jsonFileName);

	//create standard grid texture
	static TexturePtr createStandardTexture(RenderDevice* device, int32_t width=256, int32_t height=256, int32_t gridSize=64,
		const fVector3& ltColor = fVector3::RED, const fVector3& rtColor = fVector3::GREEN, 
		const fVector3& lbColor = fVector3::BLUE, const fVector3& rbColor = fVector3::PURPLE);

	//load texture from file
	static TexturePtr loadTextureFromPNG(RenderDevice* device, const char* filename);
	//save pixel buffer to file
	static bool savePixelBufferToPNG(const char* filename, int32_t width, int32_t height, const void* pixelData, PixelFormat pixelFormat);
};

}
