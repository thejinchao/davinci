#include <GLFW/glfw3.h>
#include "dc_sample.h"

//-------------------------------------------------------------------------------------
void SampleBase::dumpToGLTexture(uint32_t textureID)
{
	const auto& colorBuffer = m_renderTarget.getColorBuffer();
	uint8_t* data = new uint8_t[(size_t)(colorBuffer.getWidth()*colorBuffer.getHeight() * 3)];
	for (size_t i = 0; i < (size_t)(colorBuffer.getWidth()*colorBuffer.getHeight()); i++) {
		const fVector4* color = colorBuffer.ptr() + i;
		data[i * 3 + 0] = (uint8_t)(color->x * 255);
		data[i * 3 + 1] = (uint8_t)(color->y * 255);
		data[i * 3 + 2] = (uint8_t)(color->z * 255);
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colorBuffer.getWidth(), colorBuffer.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	delete[] data;
}

//-------------------------------------------------------------------------------------
void SampleBase::dumpToPNGFile(const char* color_filename, const char* depth_filename)
{
	//save rgba
	AssetUtility::savePixelBufferToPNG(color_filename,
		m_renderTarget.getWidth(), m_renderTarget.getHeight(),
		m_renderTarget.getColorBuffer().ptr(), PF_FLOAT32_RGBA);

	//save depth
	std::vector<float> depth_data((size_t)(m_renderTarget.getWidth() * m_renderTarget.getHeight()));
	const float* p = m_renderTarget.getDepthBuffer().ptr();
	for (size_t i = 0; i < (size_t)(m_renderTarget.getWidth() * m_renderTarget.getHeight()); i++) {
		depth_data[i] = 1.f - MathUtil::saturate(*p++);
	}
	AssetUtility::savePixelBufferToPNG(depth_filename, m_renderTarget.getWidth(), m_renderTarget.getHeight(), &(depth_data[0]), PF_FLOAT32_R);
}

