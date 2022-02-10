#include "dv_precompiled.h"
#include "dv_entity.h"

#include "dv_pipe_vs.h"
#include "dv_pipe_ps.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void EntityRenderable::build(const RenderDevice* device, const Matrix4& transform, const Model::MeshPart* meshPart, VertexShaderPtr vs, PixelShaderPtr ps)
{
	m_meshPart = meshPart;

	m_primitiveType = m_meshPart->m_primitiveType;
	m_worldTransform = transform;
	m_vs = vs;
	m_ps = ps;
	m_vsConstantBuffer = std::make_shared<ConstantBuffer>(device);
	m_psConstantBuffer = std::make_shared<ConstantBuffer>(device);
}

//-------------------------------------------------------------------------------------
ConstVertexBufferPtr EntityRenderable::getVertexBuffer(void) const
{
	return m_meshPart->m_vertexBuffer;
}

//-------------------------------------------------------------------------------------
ConstIndexBufferPtr EntityRenderable::getIndexBuffer(void) const
{
	return m_meshPart->m_indexBuffer;
}

//-------------------------------------------------------------------------------------
void Entity::build(const Matrix4& transform, ConstModelPtr model, VertexShaderPtr vs, PixelShaderPtr ps)
{
	m_transform = transform;
	m_model = model;
	m_vs = vs;
	m_ps = ps;
}

//-------------------------------------------------------------------------------------
void Entity::render(const Matrix4& transParent, RenderQueue& queue)
{
	Matrix4 transform = m_transform * transParent;

	//visit all submodel of this model
	m_model->visit(transform, [&queue, this](const Matrix4& trans, const Model::MeshPart* meshPart) {

		RenderablePtr entityRenderable = std::shared_ptr<Renderable>(new EntityRenderable());
		((EntityRenderable*)entityRenderable.get())->build(queue.getDevice(), trans, meshPart, m_vs, m_ps);

		//prepare shader
		m_vs->preRender(trans, entityRenderable);
		m_ps->preRender(entityRenderable);

		queue.pushRenderable(entityRenderable);
	});

	//call parent
	SceneObject::render(transParent, queue);
}


}

