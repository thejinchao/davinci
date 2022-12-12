#pragma once

#include "dv_prerequisites.h"

//to be remove
#include "device/dv_constant_buffer.h"
#include "device/dv_vertex_desc.h"

namespace davinci
{

class PrimitiveAfterVS
{
public:
	struct Node
	{
		DeviceBufferPtr			vertexData;
		size_t					vertexSize;
		size_t					vertexCounts;
		PrimitiveType			primitiveType;
		ConstPixelShaderPtr		ps;
		ConstConstantBufferPtr	psConstantBuffer;
		std::vector<float>		invZ;
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

class VertexShader
{
public:
	virtual void preRender(const fMatrix4& worldTransform, RenderablePtr renderable) const = 0;
	virtual void vsFunction(ConstConstantBufferPtr constantBuffer, const float* input, const VertexDesc::OffsetData& inputVertexOffset, float* output, float& invZ) const = 0;
	virtual const VertexDesc& getOutputVertexDesc(void) const = 0;

public:
	static void process(const RenderDevice* device, const PrimitiveAfterAssember& input, PrimitiveAfterVS& output);

public:
	VertexShader() {}
	virtual ~VertexShader() {}
};

}
