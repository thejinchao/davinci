#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class SceneObject
{
public:
	void addChild(SceneObjectPtr child);
	void setTransform(const fMatrix4& transform);
	virtual void render(const fMatrix4& transParent, RenderQueue& queue);

protected:
	fMatrix4 m_transform;
	SceneObjectVector m_childen;

public:
	SceneObject();
	~SceneObject();
};

}
