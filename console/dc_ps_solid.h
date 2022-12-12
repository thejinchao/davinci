#include <davinci.h>
using namespace davinci;

template<typename PSIN>
class PixelShaderSolid : public PixelShader
{
public:
	struct PSConstantBuffer
	{
		fVector3 solidColor;
	};

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		param.solidColor = m_color;
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, fVector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);

		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);

		color = fVector4(param->solidColor, 1.f);
		depth = psin->pos.z;
	}

private:
	fVector3 m_color;

public:
	PixelShaderSolid(fVector3 color) : m_color(color) { }
	~PixelShaderSolid() {}
};
