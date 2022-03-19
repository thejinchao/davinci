#include "dv_precompiled.h"
#include "dv_pipe_VS.h"

#include "dv_pipe_IA.h"
#include "device/dv_render_device.h"
#include "device/dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void VertexShader::process(const RenderDevice* device, const PrimitiveAfterAssember& input, PrimitiveAfterVS& output)
{
	input.visitor([device, &input, &output](const PrimitiveAfterAssember::Node& inputNode) {
		PrimitiveAfterVS::Node outputNode;

		outputNode.vertexSize = inputNode.vs->getOutputVertexDesc().vertexSize();
		outputNode.vertexCounts = inputNode.vertexCounts;
		outputNode.vertexData = device->createDeviceBuffer(inputNode.vertexCounts * outputNode.vertexSize*sizeof(float));
		outputNode.primitiveType = inputNode.primitiveType;
		outputNode.ps = inputNode.ps;
		outputNode.psConstantBuffer = inputNode.psConstantBuffer;
		outputNode.invZ.resize(inputNode.vertexCounts);

		Matrix4 trans = inputNode.transform;

		//vs shader
		for (size_t i = 0; i < inputNode.vertexCounts; i++) {
			const float* in = (const float*)(inputNode.vertexData->ptr(i*inputNode.vertexSize*sizeof(float)));
			float* out = (float*)(outputNode.vertexData->ptr(i*outputNode.vertexSize*sizeof(float)));
			float& invZ = outputNode.invZ[i];

			inputNode.vs->vsFunction(inputNode.vsConstantBuffer, in, inputNode.vertexElementOffset, out, invZ);
		}

		output.pushNode(outputNode);
	});
}

}
