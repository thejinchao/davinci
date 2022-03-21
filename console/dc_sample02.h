#pragma once

#include "dc_sample.h"

class Sample02 : public SampleBase
{
public:
	virtual bool init(void);
	virtual void render(int32_t width, int32_t height);

private:
	RenderDevice m_device;
	Scene  m_scene;
	VertexShaderPtr m_vs;
	PixelShaderPtr m_ps;
	Camera m_camera;

	SceneObjectPtr m_cube;
	TexturePtr m_texture;

	float m_updateT;
};
