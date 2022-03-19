#pragma once

#include "dv_prerequisites.h"

#include "dv_vertex_desc.h"

namespace davinci
{

class VertexBuffer : noncopyable
{
public:
	//build vertex buffer
	void build(RenderDevice* device, const VertexDesc& desc, float* vertices, size_t verticesCounts);

	//get vertex counts
	size_t counts(void) const;

	//get vertex size(in number of floats)
	size_t vertexSize(void) const { return m_vertexDesc.vertexSize(); }

	//get data
	const float* ptr(size_t index) const;

	//get vertex element desc
	const VertexDesc& getVertexDesc(void) const { return m_vertexDesc; }

private:
	VertexDesc m_vertexDesc;
	DeviceBufferPtr m_vertices;

public:
	VertexBuffer() {}
	~VertexBuffer() {}
};

}
