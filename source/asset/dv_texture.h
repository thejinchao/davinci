#pragma once

#include "dv_prerequisites.h"

#include "dv_asset_utility.h"

namespace davinci
{

class Texture : noncopyable
{
public:
	fVector3 getRGB(const fVector2& uv) const;

	//Pixel size(counts of loat)
	int32_t	getPixelSize(void) const {
		return m_pixelSize;
	}

	static int32_t getPixelSize(PixelFormat pixelFormat);

private:
	int32_t				m_width;
	int32_t				m_height;
	int32_t				m_pixelSize;
	PixelFormat			m_pixelFormat;
	std::vector<float>	m_pixelData;

public:
	Texture(int32_t width, int32_t height, PixelFormat pixelFormat);
	~Texture();

	friend TexturePtr AssetUtility::loadTextureFromPNG(RenderDevice* device, const char* filename);
	friend TexturePtr AssetUtility::createStandardTexture(RenderDevice* device, int32_t width, int32_t height, int32_t gridSize,
		const fVector3& ltColor, const fVector3& rtColor, const fVector3& lbColor, const fVector3& rbColor);
};

}