#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

//Description of a vertex data
class VertexDesc
{
public:
	typedef std::vector<int32_t> OffsetData;

public:
	void addElement(VertexElementType type, VertexElementFormat format);

	//return offset of element(-1 mean not exist)
	int32_t getElementOffset(VertexElementType type) const;
	const OffsetData& getElementOffset(void) const {
		return m_elementOffset;
	}
	//the size (in number of floats) of each vertex
	size_t vertexSize(void) const { return m_vertexSize; }
	//utility function: get size(in number of floats) of a element 
	static size_t elementSize(VertexElementFormat format);

private:
	struct Element
	{
		VertexElementType	type;
		VertexElementFormat format;
		size_t				offset;	//byte offset
		size_t				size;	//byte size
	};
	typedef std::vector< Element > ElementBuf;
	ElementBuf	m_elements;
	size_t		m_vertexSize;
	size_t		m_currentOffset;
	OffsetData	m_elementOffset;

public:
	VertexDesc();
	~VertexDesc() {}
};
}
