#pragma once

#include "dv_prerequisites.h"


namespace davinci
{

class PixelShader
{

public:
	virtual void preRender(RenderablePtr renderable) const = 0;
	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, Vector4& color, float& depth) const = 0;

public:
	static void process(const PrimitiveAfterVS& input, RenderTarget& output);

public:
	PixelShader() {}
	virtual ~PixelShader() {}
};

}
