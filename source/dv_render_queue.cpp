#include "dv_precompiled.h"
#include "dv_render_queue.h"

#include "device/dv_constant_buffer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void Renderable::setVSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length)
{
	m_vsConstantBuffer->setBuffer(index, buffer, length);
}

//-------------------------------------------------------------------------------------
void Renderable::setPSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length)
{
	m_psConstantBuffer->setBuffer(index, buffer, length);
}

//-------------------------------------------------------------------------------------
void RenderQueue::clear(void)
{
	m_queue.clear();
}

//-------------------------------------------------------------------------------------
void RenderQueue::setCamera(const Camera& camera)
{
	m_camera = camera;
}

//-------------------------------------------------------------------------------------
void RenderQueue::pushRenderable(RenderablePtr renderable)
{
	m_queue.push_back(renderable);
}

//-------------------------------------------------------------------------------------
void RenderQueue::visitorRenderable(std::function<void(ConstRenderablePtr renderable)> visitorFunc) const
{
	for (ConstRenderablePtr renderable : m_queue) {
		visitorFunc(renderable);
	}
}

}

