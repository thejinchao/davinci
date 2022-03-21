#include "dc_sample01.h"

#include "dc_ps_dirlight.h"
#include "dc_ps_solid.h"
#include "dc_vs_standard.h"

struct VSOUT
{
	Vector3 pos;
	Vector3 normal;
};

#define LIGHT_COUNTS 2
typedef VertexShaderStandard<true, false>			VSStandard;
typedef PixelShaderStandard<VSOUT, LIGHT_COUNTS>	PSWithDirLight;

//-------------------------------------------------------------------------------------
bool Sample01::init(void)
{
	//create scene
	m_scene.init();

	//create shader
	VertexDesc vsoutDesc;
	vsoutDesc.addElement(VertexElementType::VET_POSITION, VET_FLOAT_X3);
	vsoutDesc.addElement(VertexElementType::VET_NORMAL, VET_FLOAT_X3);

	m_vs = std::make_shared<VSStandard>(vsoutDesc, &m_camera);
	m_ps = std::make_shared<PSWithDirLight>();
	
	m_lightColor[0] = Vector3(0.5f, 0.5f, 0.5f);
	m_lightColor[1] = Vector3(0.5f, 0.0f, 0.0f);
	((PSWithDirLight*)(m_ps.get()))->setLightColor(0, m_lightColor[0]);
	((PSWithDirLight*)(m_ps.get()))->setLightColor(1, m_lightColor[1]);

	//build a scene
	Entity* entity_cube = new Entity();
	entity_cube->build(Matrix4::IDENTITY,
		AssetUtility::createStandardModel_Box(&m_device, 1.f, 1.f, 1.f, PrimitiveType::PT_TRIANGLE_LIST, true, true, true),
		m_vs, m_ps);

	m_cube = std::shared_ptr<SceneObject>((SceneObject*)entity_cube);
	m_scene.addNode(m_cube);

	Entity* entity_light1 = new Entity();
	entity_light1->build(Matrix4::IDENTITY,
		AssetUtility::createStandardModel_Box(&m_device, 0.2f, 0.2f, 0.2f, PrimitiveType::PT_TRIANGLE_LIST, true, true, true),
		m_vs, 
		std::make_shared<PixelShaderSolid<VSOUT>>(m_lightColor[0]));
	m_light1 = std::shared_ptr<SceneObject>((SceneObject*)entity_light1);
	m_scene.addNode(m_light1);


	Entity* entity_light2 = new Entity();
	entity_light2->build(Matrix4::IDENTITY,
		AssetUtility::createStandardModel_Box(&m_device, 0.2f, 0.2f, 0.2f, PrimitiveType::PT_TRIANGLE_LIST, true, true, true),
		m_vs,
		std::make_shared<PixelShaderSolid<VSOUT>>(m_lightColor[1]));
	m_light2 = std::shared_ptr<SceneObject>((SceneObject*)entity_light2);
	m_scene.addNode(m_light2);

	m_rotateParam = 0.f;
	return true;
}

//-------------------------------------------------------------------------------------
void Sample01::render(int32_t width, int32_t height)
{
	//update scene
	m_rotateParam += MathUtil::PI / 500.f;

	// Setup our lighting parameters
	Vector3 vLightDirs[2] =
	{
		Vector3(-0.577f, 0.577f, -0.577f),
		Vector3(0.0f, 0.0f, -1.0f),
	};

	// Rotate the second light around the origin
	m_lightDir[0] = vLightDirs[0];
	m_lightDir[1] = vLightDirs[1] * Matrix4::makeRotate_Y(2.f*m_rotateParam);

	//rotate main cube
	m_cube->setTransform(Matrix4::makeRotate_Y(-m_rotateParam));
	//move light cube
	m_light1->setTransform(Matrix4::makeTrans(m_lightDir[0] * 5));
	m_light2->setTransform(Matrix4::makeTrans(m_lightDir[1] * 5));

	//update shader
	((PSWithDirLight*)(m_ps.get()))->setLightDir(0, m_lightDir[0]);
	((PSWithDirLight*)(m_ps.get()))->setLightDir(1, m_lightDir[1]);

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

