#include "dv_precompiled.h"
#include "dv_renderable.h"

#include "device/dv_constant_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void Renderable::setVSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length)
{
	m_vsConstantBuffer->setBuffer(index, buffer, length);
}

//-------------------------------------------------------------------------------------
void Renderable::setPSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length)
{
	m_psConstantBuffer->setBuffer(index, buffer, length);
}

}
