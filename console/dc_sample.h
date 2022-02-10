#pragma once

#include <davinci.h>
using namespace davinci;

class SampleBase
{
public:
	void dumpToGLTexture(uint32_t textureID);
	void dumpToPNGFile(const char* color_filename, const char* depth_filename);

protected:
	RenderTarget m_renderTarget;
};
