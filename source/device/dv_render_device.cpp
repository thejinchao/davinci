#include "dv_precompiled.h"
#include "dv_render_device.h"
#include "dv_device_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
DeviceBufferPtr RenderDevice::createDeviceBuffer(size_t size) const
{
	DeviceBufferPtr deviceBuffer = std::make_shared<DeviceBuffer>();
	deviceBuffer->init(size);

	return deviceBuffer;
}

}