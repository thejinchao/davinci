#include "dv_precompiled.h"
#include "dv_vertex_desc.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
VertexDesc::VertexDesc() 
	: m_vertexSize(0)
	, m_currentOffset(0) 
{
	m_elementOffset.resize((size_t)VertexElementType::VET_COUNTS, -1);
}

//-------------------------------------------------------------------------------------
void VertexDesc::addElement(VertexElementType type, VertexElementFormat format)
{
	m_elementOffset[(size_t)type] = (int32_t)m_currentOffset;

	Element element;
	element.type = type;
	element.format = format;
	element.offset = m_currentOffset;
	element.size = elementSize(format);
	m_currentOffset += element.size;
	m_vertexSize += element.size;

	m_elements.push_back(element);
}

//-------------------------------------------------------------------------------------
int32_t VertexDesc::getElementOffset(VertexElementType type) const
{
	assert((size_t)type >= 0 && type < VertexElementType::VET_COUNTS);

	return m_elementOffset[(size_t)type];
}

//-------------------------------------------------------------------------------------
size_t VertexDesc::elementSize(VertexElementFormat format)
{
	switch (format)
	{
	case VertexElementFormat::VEF_FLOAT_X1:
	case VertexElementFormat::VET_UINT32_X1:
		return 1;

	case VertexElementFormat::VET_FLOAT_X2:
	case VertexElementFormat::VET_UINT32_X2:
		return 2;

	case VertexElementFormat::VET_FLOAT_X3:
	case VertexElementFormat::VET_UINT32_X3:
		return 3;

	case VertexElementFormat::VET_FLOAT_X4:
	case VertexElementFormat::VET_UINT32_X4:
		return 4;

	default:
		assert(false && "Unknown VertexElementFormat format");
		return 0;
	}
}
}
