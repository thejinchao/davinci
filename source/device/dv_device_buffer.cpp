#include "dv_precompiled.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
DeviceBuffer::DeviceBuffer()
	: m_ptr(nullptr)
	, m_size(0)
{
}

//-------------------------------------------------------------------------------------
DeviceBuffer::~DeviceBuffer()
{
	clear();
}

//-------------------------------------------------------------------------------------
void DeviceBuffer::init(size_t size)
{
	assert(size > 0);
	clear();

	m_ptr = (uint8_t*)malloc(size);
	memset(m_ptr, 0, size);
	m_size = size;
}

//-------------------------------------------------------------------------------------
void DeviceBuffer::clear(void)
{
	if (m_ptr) {
		free(m_ptr);
		m_ptr = nullptr;
	}
	m_size = 0;
}

}