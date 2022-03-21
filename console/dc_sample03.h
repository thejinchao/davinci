#pragma once
#include "dc_sample.h"

class Sample03 : public SampleBase
{
public:
	virtual bool init(void);
	virtual void render(int32_t width, int32_t height);

	const Camera& getCamera(void) const { return m_camera; }

private:
	RenderDevice m_device;
	Scene  m_scene;
	VertexShaderPtr m_vs;
	PixelShaderPtr m_ps;
	Camera m_camera;

	SceneObjectPtr m_model;

	float m_rotateParam;
};
