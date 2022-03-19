#pragma once

#include "dv_prerequisites.h"

//to be remove
#include "scene/dv_camera.h"

namespace davinci
{

class Renderable
{
public:
	virtual ConstVertexBufferPtr getVertexBuffer(void) const = 0;
	virtual ConstIndexBufferPtr getIndexBuffer(void) const = 0;

	virtual PrimitiveType getPrimitiveType(void) const {
		return m_primitiveType;
	}
	
	const Matrix4& getWorldTransform(void) const {
		return m_worldTransform;
	}
	
	ConstVertexShaderPtr getVS(void) const {
		return m_vs;
	}
	
	ConstPixelShaderPtr getPS(void) const {
		return m_ps;
	}

	void setVSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length);
	void setPSConstantBuffer(int32_t index, const uint8_t* buffer, size_t length);

	ConstConstantBufferPtr getVSConstantBuffer(void) const {
		return m_vsConstantBuffer;
	}
	ConstConstantBufferPtr getPSConstantBuffer(void) const {
		return m_psConstantBuffer;
	}

protected:
	PrimitiveType m_primitiveType;
	Matrix4 m_worldTransform;
	ConstVertexShaderPtr m_vs;
	ConstPixelShaderPtr m_ps;
	ConstantBufferPtr m_vsConstantBuffer;
	ConstantBufferPtr m_psConstantBuffer;
};

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

protected:
	std::vector<ConstRenderablePtr> m_queue;
	const RenderDevice* m_device;
	Camera m_camera;
};

}
