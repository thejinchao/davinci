#pragma once

#include "dv_config.h"

// Pre-declare classes
// Allows use of pointers in header files without including individual .h
// so decreases dependencies between files
namespace davinci
{
class RenderQueue;
class Renderable;
class VertexShader;
class PixelShader;
class ConstantBuffer;
class DeviceBuffer;
class RenderDevice;
class VertexDesc;
class VertexBuffer;
class IndexBuffer;
class Model;
class Texture;
class SceneObject;
class Camera;
class PrimitiveAfterAssember;
class PrimitiveAfterVS;
class RenderTarget;

typedef std::shared_ptr<Renderable>				RenderablePtr;
typedef std::shared_ptr<const Renderable>		ConstRenderablePtr;
typedef std::shared_ptr<VertexShader>			VertexShaderPtr;
typedef std::shared_ptr<PixelShader>			PixelShaderPtr;
typedef std::shared_ptr<const VertexShader>		ConstVertexShaderPtr;
typedef std::shared_ptr<const PixelShader>		ConstPixelShaderPtr;
typedef std::shared_ptr<ConstantBuffer>			ConstantBufferPtr;
typedef std::shared_ptr<const ConstantBuffer>	ConstConstantBufferPtr;
typedef std::shared_ptr<DeviceBuffer>			DeviceBufferPtr;
typedef std::shared_ptr<VertexBuffer>			VertexBufferPtr;
typedef std::shared_ptr<const VertexBuffer>		ConstVertexBufferPtr;
typedef std::shared_ptr<IndexBuffer>			IndexBufferPtr;
typedef std::shared_ptr<const IndexBuffer>		ConstIndexBufferPtr;
typedef std::shared_ptr<Model>					ModelPtr;
typedef std::shared_ptr<const Model>			ConstModelPtr;
typedef std::shared_ptr<Texture>				TexturePtr;
typedef std::shared_ptr<SceneObject>			SceneObjectPtr;
typedef std::vector<SceneObjectPtr>				SceneObjectVector;
typedef std::shared_ptr<Camera>					CameraPtr;

typedef std::vector<DeviceBufferPtr>			DeviceBufferVector;

}

#include "dv_precompiled.h"

namespace davinci
{

	class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:
	noncopyable(const noncopyable&) = delete;
	const noncopyable& operator=(const noncopyable&) = delete;
};

}
