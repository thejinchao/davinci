#include <davinci.h>
using namespace davinci;

template<typename PSIN, size_t kLightCounts>
class PixelShaderStandard : public PixelShader
{
public:
	TexturePtr sampler;
	struct PSConstantBuffer
	{
		Vector3 lightDir[kLightCounts];
		Vector3 lightColor[kLightCounts];
	};

	void setLightColor(size_t index, const Vector3& lightColor) {
		assert(index >= 0 && index < kLightCounts);

		m_lightColor[index] = lightColor;
	}

	void setLightDir(size_t index, const Vector3& lightDir) {
		assert(index >= 0 && index < kLightCounts);

		m_lightDir[index] = lightDir;
	}

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		for (size_t i = 0; i < kLightCounts; i++) {
			param.lightDir[i] = m_lightDir[i];
			param.lightColor[i] = m_lightColor[i];
		}
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, Vector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);

		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);

		Vector3 finalColor = Vector3::ZERO;

		//do NdotL lighting for all lights
		for (int i = 0; i<2; i++)
		{
			finalColor += (param->lightDir[i].dotProduct(psin->normal) * param->lightColor[i]).saturate();
		}
		color = Vector4(finalColor, 1.f);

		depth = psin->pos.z;
	}

private:
	Vector3 m_lightDir[kLightCounts];
	Vector3 m_lightColor[kLightCounts];

public:
	PixelShaderStandard() {}
	~PixelShaderStandard() {}
};
