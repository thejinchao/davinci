#pragma once
#include "dc_sample.h"

class Sample01 : public SampleBase
{
public:
	virtual bool init(void);
	virtual void render(int32_t width, int32_t height);

	const Camera& getCamera(void) const { return m_camera; }
	const fVector3& getLightDir(size_t index) const { return m_lightDir[index]; }
	const fVector3& getLightColor(size_t index) const { return m_lightColor[index]; }

private:
	RenderDevice m_device;
	Scene  m_scene;
	VertexShaderPtr m_vs;
	PixelShaderPtr m_ps;
	Camera m_camera;

	SceneObjectPtr m_cube, m_light1, m_light2;
	fVector3 m_lightDir[2];
	fVector3 m_lightColor[2];

	float m_rotateParam;
};
