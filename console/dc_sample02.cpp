#include "dc_sample02.h"

#include "dc_ps_texture.h"
#include "dc_vs_standard.h"

struct VSOUT
{
	Vector3 pos;
	Vector2 uv0;
};

typedef VertexShaderStandard<false, true>	VSStandard;
typedef PixelShaderTexture<VSOUT>			PSWithTexture;

//-------------------------------------------------------------------------------------
bool Sample02::init(void)
{
	//create scene
	m_scene.init();

	//create shader
	VertexDesc vsoutDesc;
	vsoutDesc.addElement(VertexElementType::VET_POSITION, VET_FLOAT_X3);
	vsoutDesc.addElement(VertexElementType::VET_TEXCOORD0, VET_FLOAT_X2);

	m_texture = AssetUtility::createStandardTexture(&m_device, 256, 256); 

	m_vs = std::make_shared<VSStandard>(vsoutDesc, &m_camera);
	m_ps = std::make_shared<PSWithTexture>(m_texture);

	//build a scene
	Entity* entity_cube = new Entity();
	entity_cube->build(Matrix4::IDENTITY,
		AssetUtility::createStandardModel_Box(&m_device, 1.f, 1.f, 1.f, PrimitiveType::PT_TRIANGLE_LIST, true, true, true),
		m_vs, m_ps);

	m_cube = std::shared_ptr<SceneObject>((SceneObject*)entity_cube);
	m_scene.addNode(m_cube);

	m_updateT = 0.f;
	return true;
}

//-------------------------------------------------------------------------------------
void Sample02::render(int32_t width, int32_t height)
{
	//set mesh color
	m_updateT += MathUtil::PI * 0.0125f;

	Vector3 meshColor;
	meshColor.x = (sinf(m_updateT * 1.0f) + 1.0f) * 0.5f;
	meshColor.y = (cosf(m_updateT * 3.0f) + 1.0f) * 0.5f;
	meshColor.z = (sinf(m_updateT * 5.0f) + 1.0f) * 0.5f;
	((PSWithTexture*)m_ps.get())->setMeshColor(meshColor);

	//set camera
	m_camera.setEye(Vector3(4.f, 3.f, -6.f), false);
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
