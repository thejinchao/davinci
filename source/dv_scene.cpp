#include "dv_precompiled.h"
#include "dv_scene.h"

#include "dv_scene_object.h"
#include "dv_render_queue.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
Scene::Scene()
{
}

//-------------------------------------------------------------------------------------
Scene::~Scene()
{
}

//-------------------------------------------------------------------------------------
void Scene::init(void)
{
	m_root = nullptr; //clear

	//set a new root
	m_root = std::shared_ptr<SceneObject>(new SceneObject());
}

//-------------------------------------------------------------------------------------
void Scene::addNode(SceneObjectPtr object, SceneObjectPtr parent)
{
	if (parent == nullptr) {
		parent = m_root;
	}
	parent->addChild(object);
}

//-------------------------------------------------------------------------------------
void Scene::render(const RenderDevice& device, const Camera& camera, RenderQueue& renderQueue)
{
	renderQueue.setDevice(&device);
	renderQueue.setCamera(camera);
	m_root->render(Matrix4::IDENTITY, renderQueue);
}

}

