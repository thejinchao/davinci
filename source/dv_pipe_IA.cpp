#include "dv_precompiled.h"
#include "dv_pipe_IA.h"

#include "dv_render_queue.h"
#include "dv_vertex_buffer.h"
#include "dv_index_buffer.h"
#include "dv_render_device.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void InputAssember::process(const RenderQueue& renderQueue, PrimitiveAfterAssember& output)
{
	renderQueue.visitorRenderable([&renderQueue, &output](ConstRenderablePtr renderable) {
		switch (renderable->getPrimitiveType()) {
		case PT_POINT_LIST:
		case PT_LINE_LIST:
		case PT_TRIANGLE_LIST:
		{
			PrimitiveAfterAssember::Node node;
			size_t vertexSize = renderable->getVertexBuffer()->vertexSize();

			node.transform = renderable->getWorldTransform();
			node.primitiveType = renderable->getPrimitiveType();
			node.vertexSize = renderable->getVertexBuffer()->vertexSize();
			node.vertexCounts = renderable->getIndexBuffer()->counts();
			node.vertexData = renderQueue.getDevice()->createDeviceBuffer(node.vertexCounts*vertexSize*sizeof(float));
			node.vs = renderable->getVS();
			node.ps = renderable->getPS();
			node.vsConstantBuffer = renderable->getVSConstantBuffer();
			node.psConstantBuffer = renderable->getPSConstantBuffer();
			node.vertexElementOffset = renderable->getVertexBuffer()->getVertexDesc().getElementOffset();

			//copy vertex data
			for (size_t i = 0; i < node.vertexCounts; i++) {

				uint16_t index = renderable->getIndexBuffer()->get(i);
				//vertex data
				memcpy(node.vertexData->ptr(i*vertexSize*sizeof(float)), renderable->getVertexBuffer()->ptr(index), vertexSize * sizeof(float));
			}


			output.pushNode(node);
		}
		break;

		default:
			break;
		}
	});
}

}
