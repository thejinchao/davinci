#include "dv_precompiled.h"
#include "dv_vertex_buffer.h"

#include "dv_render_device.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void VertexBuffer::build(RenderDevice* device, const VertexDesc& desc, float* vertices, size_t verticesCounts)
{
	assert(vertices && verticesCounts>0);

	m_vertexDesc = desc;
	m_vertices = device->createDeviceBuffer(desc.vertexSize() * verticesCounts * sizeof(float));

	memcpy(m_vertices->ptr(0), vertices, desc.vertexSize() * verticesCounts * sizeof(float));
}

//-------------------------------------------------------------------------------------
size_t VertexBuffer::counts(void) const 
{ 
	return m_vertices->size() / vertexSize(); 
}

//-------------------------------------------------------------------------------------
const float* VertexBuffer::ptr(size_t index) const 
{
	return (const float*)(m_vertices->ptr(index*vertexSize() * sizeof(float)));
}

}
