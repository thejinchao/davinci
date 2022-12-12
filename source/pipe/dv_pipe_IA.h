#pragma once

#include "dv_prerequisites.h"

//to be remove
#include "device/dv_vertex_desc.h"
#include "device/dv_constant_buffer.h"

namespace davinci
{

class PrimitiveAfterAssember
{
public:
	struct Node
	{
		fMatrix4					transform;
		DeviceBufferPtr			vertexData;
		size_t					vertexSize;
		size_t					vertexCounts;
		VertexDesc::OffsetData	vertexElementOffset;
		PrimitiveType			primitiveType;
		ConstVertexShaderPtr	vs;
		ConstPixelShaderPtr		ps;
		ConstConstantBufferPtr	vsConstantBuffer;
		ConstConstantBufferPtr	psConstantBuffer;
	};

	void pushNode(Node& node) {
		m_primitives.push_back(node);
	}

	void visitor(std::function<void(const Node&)> visitorFunc) const {
		for (const Node& node : m_primitives) {
			visitorFunc(node);
		}
	}
private:
	std::vector<Node> m_primitives;
};

class InputAssember
{
public:
	static void process(const RenderQueue& renderQueue, PrimitiveAfterAssember& output);
};


}
