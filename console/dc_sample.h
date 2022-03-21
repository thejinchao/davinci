#pragma once

#include <davinci.h>
using namespace davinci;

class SampleBase
{
public:
	virtual bool init(void) = 0;
	virtual void render(int32_t width, int32_t height) = 0;

	void dumpToGLTexture(uint32_t textureID);
	void dumpToPNGFile(const char* color_filename, const char* depth_filename);

protected:
	RenderTarget m_renderTarget;
};
