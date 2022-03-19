#include "dv_precompiled.h"
#include "dv_index_buffer.h"

#include "dv_render_device.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void IndexBuffer::build(const RenderDevice* device, uint16_t* indices, size_t indexCounts)
{
	assert(indices && indexCounts>0);

	m_indices = device->createDeviceBuffer(indexCounts*sizeof(uint16_t));

	memcpy(m_indices->ptr(0), indices, sizeof(uint16_t) * indexCounts);
}

//-------------------------------------------------------------------------------------
size_t IndexBuffer::counts(void) const 
{ 
	return m_indices->size() / sizeof(uint16_t);
}

//-------------------------------------------------------------------------------------
uint16_t IndexBuffer::get(size_t index) const 
{ 
	return *((const uint16_t*)m_indices->ptr(index * sizeof(uint16_t)));
}

}
