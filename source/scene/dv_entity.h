#pragma once

#include "dv_prerequisites.h"

#include "dv_scene_object.h"
#include "pipe/dv_render_queue.h"
#include "pipe/dv_renderable.h"

//to be remove
#include "asset/dv_model.h"

namespace davinci
{

class EntityRenderable : public Renderable
{
public:
	void build(const RenderDevice* device, const fMatrix4& transform, const Model::MeshPart* meshPart, VertexShaderPtr vs, PixelShaderPtr ps);

	virtual ConstVertexBufferPtr getVertexBuffer(void) const;
	virtual ConstIndexBufferPtr getIndexBuffer(void) const;

protected:
	const Model::MeshPart* m_meshPart;
};

class Entity : public SceneObject
{
public:
	void build(const fMatrix4& transform, ConstModelPtr model, VertexShaderPtr vs, PixelShaderPtr ps);
	virtual void render(const fMatrix4& transParent, RenderQueue& queue);

private:
	ConstModelPtr m_model;
	VertexShaderPtr m_vs;
	PixelShaderPtr m_ps;
};

}
