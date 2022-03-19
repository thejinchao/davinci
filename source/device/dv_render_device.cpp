#include "dv_precompiled.h"
#include "dv_render_device.h"

#include "dv_device_buffer.h"
#include "dv_pipe_IA.h"
#include "dv_pipe_VS.h"
#include "dv_pipe_PS.h"
#include "dv_render_queue.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
DeviceBufferPtr RenderDevice::createDeviceBuffer(size_t size) const
{
	DeviceBufferPtr deviceBuffer = std::make_shared<DeviceBuffer>();
	deviceBuffer->init(size);

	return deviceBuffer;
}

//-------------------------------------------------------------------------------------
void RenderDevice::process(const RenderQueue& renderQueue, RenderTarget& renderTarget)
{
	//0 : Input Assember
	PrimitiveAfterAssember inputPrimitive;
	InputAssember::process(renderQueue, inputPrimitive);

	//1 : Vertex Shader
	PrimitiveAfterVS primitiveAfterVS;
	VertexShader::process(renderQueue.getDevice(), inputPrimitive, primitiveAfterVS);

	//2: Pixel Shader
	PixelShader::process(primitiveAfterVS, renderTarget);
}

}