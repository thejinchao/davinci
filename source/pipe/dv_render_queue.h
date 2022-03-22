#pragma once

#include "dv_prerequisites.h"

//to be remove
#include "scene/dv_camera.h"

namespace davinci
{

class RenderQueue
{
public:
	void clear(void);
	
	void setCamera(const Camera& camera);
	const Camera& getCamera(void) const {
		return m_camera; 
	}

	void setDevice(const RenderDevice* device) {
		m_device = device;
	}
	const RenderDevice* getDevice(void) const {
		return m_device;
	}

	void pushRenderable(RenderablePtr renderable);
	void visitorRenderable(std::function<void(ConstRenderablePtr renderable)> visitorFunc) const;

	void process(RenderTarget& renderTarget);

protected:
	std::vector<ConstRenderablePtr> m_queue;
	const RenderDevice* m_device;
	Camera m_camera;
};

}
