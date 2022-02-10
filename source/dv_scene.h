#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class Scene
{
public:
	void init(void);
	void addNode(SceneObjectPtr object, SceneObjectPtr parent=nullptr);
	void render(const RenderDevice& device, const Camera& camera, RenderQueue& renderQueue);

protected:
	SceneObjectPtr m_root;

public:
	Scene();
	~Scene();
};

}
