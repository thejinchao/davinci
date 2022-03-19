#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class DeviceBuffer
{
public:
	void init(size_t size);
	void clear(void);

	size_t size(void) const { return m_size; }

	const uint8_t* ptr(size_t offseet) const {
		return m_ptr + offseet;
	}
	uint8_t* ptr(size_t offseet) {
		return m_ptr + offseet;
	}
private:
	uint8_t*	m_ptr;
	size_t		m_size;

public:
	DeviceBuffer();
	~DeviceBuffer();
};

}
