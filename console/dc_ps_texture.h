#include <davinci.h>
using namespace davinci;

template<typename PSIN>
class PixelShaderTexture : public PixelShader
{
public:
	struct PSConstantBuffer
	{
		fVector3 meshColor;
	};

public:
	void setMeshColor(const fVector3& meshColor) {
		m_color = meshColor;
	}

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		param.meshColor = m_color;
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, fVector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);
		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);
		const fVector3 meshColor = param->meshColor;

		color = fVector4(m_texture->getRGB(psin->uv0)*meshColor, 1.f);
		depth = psin->pos.z;
	}

private:
	TexturePtr m_texture;
	fVector3 m_color;

public:
	PixelShaderTexture(TexturePtr texture) : m_texture(texture), m_color(fVector3::WHITE) { }
	~PixelShaderTexture() {}
};
