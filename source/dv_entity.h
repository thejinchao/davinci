#pragma once

#include "dv_prerequisites.h"

#include "dv_scene_object.h"
#include "dv_render_queue.h"

//to be remove
#include "dv_model.h"

namespace davinci
{

class EntityRenderable : public Renderable
{
public:
	void build(const RenderDevice* device, const Matrix4& transform, const Model::MeshPart* meshPart, VertexShaderPtr vs, PixelShaderPtr ps);

	virtual ConstVertexBufferPtr getVertexBuffer(void) const;
	virtual ConstIndexBufferPtr getIndexBuffer(void) const;

protected:
	const Model::MeshPart* m_meshPart;
};

class Entity : public SceneObject
{
public:
	void build(const Matrix4& transform, ConstModelPtr model, VertexShaderPtr vs, PixelShaderPtr ps);
	virtual void render(const Matrix4& transParent, RenderQueue& queue);

private:
	ConstModelPtr m_model;
	VertexShaderPtr m_vs;
	PixelShaderPtr m_ps;
};

}
