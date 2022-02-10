#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class IndexBuffer : noncopyable
{
public:
	void build(const RenderDevice* device, uint16_t* indices, size_t indexCounts);

	//get index counts
	size_t counts(void) const;
	//get data
	uint16_t get(size_t index) const;

private:
	DeviceBufferPtr m_indices;

public:
	IndexBuffer() {}
	~IndexBuffer() {}
};

}
