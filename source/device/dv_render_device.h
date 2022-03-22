#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class RenderDevice
{
public:
	DeviceBufferPtr createDeviceBuffer(size_t size) const;

private:
};

}