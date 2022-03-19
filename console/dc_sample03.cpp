#include "dc_sample03.h"

#include "dc_vs_standard.h"
#include "dc_ps_material.h"

struct VSOUT
{
	Vector3 pos;
	Vector3 normal;
	//Vector2 uv0;
};

typedef VertexShaderStandard<true, false>	VSStandard;
typedef PixelShaderMaterial<VSOUT>			PSMaterial;

//-------------------------------------------------------------------------------------
void Sample03::init(void)
{
	//create scene
	m_scene.init();

	//create shader
	VertexDesc vsoutDesc;
	vsoutDesc.addElement(VertexElementType::VET_POSITION, VET_FLOAT_X3);
	vsoutDesc.addElement(VertexElementType::VET_NORMAL, VET_FLOAT_X3);
	//vsoutDesc.addElement(VertexElementType::VET_TEXCOORD0, VET_FLOAT_X2);

	m_vs = std::make_shared<VSStandard>(vsoutDesc, &m_camera);
	m_ps = std::make_shared<PSMaterial>();

	((PSMaterial*)m_ps.get())->setLightDir(Vector3(-3, -4, 10).normalise());
	((PSMaterial*)m_ps.get())->setLightColor(Vector3::WHITE);

	//build a scene
	Entity* entity = new Entity();
	entity->build(Matrix4::IDENTITY,
		//AssetUtility::createStandardModel_Sphere(&m_device, 100.f, PrimitiveType::PT_TRIANGLE_LIST),
		AssetUtility::loadModel(&m_device, "mesh/sm_matpreviewmesh_02.dam"),
		m_vs, m_ps);

	m_model = std::shared_ptr<SceneObject>((SceneObject*)entity);
	m_scene.addNode(m_model);

}

//-------------------------------------------------------------------------------------
void Sample03::render(int32_t width, int32_t height)
{
	//update scene
	m_rotateParam += MathUtil::PI / 500.f;
	m_model->setTransform(Matrix4::makeScale(0.02f, 0.02f, 0.02f) * Matrix4::makeRotate_X(-MathUtil::PI_DIV2) * Matrix4::makeRotate_Y(m_rotateParam) * Matrix4::makeTrans(0, -1.f, 0));
	//m_model->setTransform(Matrix4::makeScale(2.0f, 2.0f, 2.0f));

	//set camera
	m_camera.setEye(Vector3(0.f, 4.f, -10.f), false);
	m_camera.setLookat(Vector3(0.f, 1.f, 0.f), false);
	m_camera.setUp(Vector3::UNIT_Y, false);
	m_camera.setFov(MathUtil::PI_DIV4, false);
	m_camera.setClipRange(0.01f, 100.0f, false);
	m_camera.setAspect(width / (float)height);

	RenderQueue renderQueue;
	m_scene.render(m_device, m_camera, renderQueue);

	//render!
	m_renderTarget.init(width, height);
	m_device.process(renderQueue, m_renderTarget);
}

