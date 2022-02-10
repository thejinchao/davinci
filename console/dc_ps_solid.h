#include <davinci.h>
using namespace davinci;

template<typename PSIN>
class PixelShaderSolid : public PixelShader
{
public:
	struct PSConstantBuffer
	{
		Vector3 solidColor;
	};

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		param.solidColor = m_color;
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, Vector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);

		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);

		color = Vector4(param->solidColor, 1.f);
		depth = psin->pos.z;
	}

private:
	Vector3 m_color;

public:
	PixelShaderSolid(Vector3 color) : m_color(color) { }
	~PixelShaderSolid() {}
};
