#include <davinci.h>
using namespace davinci;

template<typename PSIN>
class PixelShaderMaterial : public PixelShader
{
public:
	TexturePtr sampler;
	struct PSConstantBuffer
	{
		fVector3 lightDir;
		fVector3 lightColor;
	};

	void setLightColor(const fVector3& lightColor) {
		m_lightColor = lightColor;
	}

	void setLightDir(const fVector3& lightDir) {

		m_lightDir = lightDir;
	}

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		param.lightDir = m_lightDir;
		param.lightColor = m_lightColor;
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, fVector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);

		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);

		fVector3 finalColor = fVector3(0.1f, 0.1f, 0.1f);

		//do NdotL lighting for all lights
		finalColor += (-param->lightDir.dotProduct(psin->normal) * param->lightColor);

		color = fVector4(finalColor.saturate(), 1.f);

		depth = psin->pos.z;
	}

private:
	fVector3 m_lightDir;
	fVector3 m_lightColor;

public:
	PixelShaderMaterial() {}
	~PixelShaderMaterial() {}
};
