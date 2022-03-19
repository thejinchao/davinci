#include "dv_precompiled.h"
#include "dv_scene_object.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
SceneObject::SceneObject()
	: m_transform(Matrix4::IDENTITY)
{

}

//-------------------------------------------------------------------------------------
SceneObject::~SceneObject()
{

}

//-------------------------------------------------------------------------------------
void SceneObject::addChild(SceneObjectPtr child)
{
	m_childen.push_back(child);
}

//-------------------------------------------------------------------------------------
void SceneObject::setTransform(const Matrix4& transform)
{
	m_transform = transform;
}

//-------------------------------------------------------------------------------------
void SceneObject::render(const Matrix4& transParent, RenderQueue& queue)
{
	Matrix4 transform = m_transform * transParent;
	for (auto child : m_childen) {
		child->render(transform, queue);
	}
}


}
