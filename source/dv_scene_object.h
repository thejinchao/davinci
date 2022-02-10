#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class SceneObject
{
public:
	void addChild(SceneObjectPtr child);
	void setTransform(const Matrix4& transform);
	virtual void render(const Matrix4& transParent, RenderQueue& queue);

protected:
	Matrix4 m_transform;
	SceneObjectVector m_childen;

public:
	SceneObject();
	~SceneObject();
};

}
