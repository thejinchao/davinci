#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class ConstantBuffer
{
public:
	enum { MAX_BUFFER_COUNTS = 16 };
	void setBuffer(int32_t index, const uint8_t* buffer, size_t length);
	const uint8_t* getBuffer(int32_t index) const;

private:
	const RenderDevice* m_device;
	DeviceBufferVector m_constantBuffer;

public:
	ConstantBuffer(const RenderDevice* device);
	~ConstantBuffer();
};


}