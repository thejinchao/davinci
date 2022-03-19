#include "dv_precompiled.h"
#include "dv_constant_buffer.h"

#include "dv_render_device.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer(const RenderDevice* device)
	: m_device(device)
{
	m_constantBuffer.resize(MAX_BUFFER_COUNTS);
}

//-------------------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{

}

//-------------------------------------------------------------------------------------
void ConstantBuffer::setBuffer(int32_t index, const uint8_t* buffer, size_t length)
{
	assert(index >= 0 && index < MAX_BUFFER_COUNTS);
	if (index < 0 || index >= MAX_BUFFER_COUNTS) return;

	DeviceBufferPtr deviceBuffer = m_constantBuffer[(size_t)index] = m_device->createDeviceBuffer(length);

	memcpy(deviceBuffer->ptr(0), buffer, length);
}

//-------------------------------------------------------------------------------------
const uint8_t* ConstantBuffer::getBuffer(int32_t index) const
{
	assert(index >= 0 && index < MAX_BUFFER_COUNTS);
	if (index < 0 || index >= MAX_BUFFER_COUNTS) return nullptr;

	DeviceBufferPtr deviceBuffer = m_constantBuffer[(size_t)index];
	return deviceBuffer->ptr(0);
}

}