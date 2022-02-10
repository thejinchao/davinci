#include <davinci.h>
using namespace davinci;

template<typename PSIN>
class PixelShaderTexture : public PixelShader
{
public:
	struct PSConstantBuffer
	{
		Vector3 meshColor;
	};

public:
	void setMeshColor(const Vector3& meshColor) {
		m_color = meshColor;
	}

public:
	virtual void preRender(RenderablePtr renderable) const {
		PSConstantBuffer param;
		param.meshColor = m_color;
		renderable->setPSConstantBuffer(0, (const uint8_t*)&param, sizeof(PSConstantBuffer));
	}

	virtual void psFunction(ConstConstantBufferPtr constantBuffer, const float* input, Vector4& color, float& depth) const {
		const PSIN* psin = (const PSIN*)(input);
		const PSConstantBuffer* param = (const PSConstantBuffer*)constantBuffer->getBuffer(0);
		const Vector3 meshColor = param->meshColor;

		color = Vector4(m_texture->getRGB(psin->uv0)*meshColor, 1.f);
		depth = psin->pos.z;
	}

private:
	TexturePtr m_texture;
	Vector3 m_color;

public:
	PixelShaderTexture(TexturePtr texture) : m_texture(texture), m_color(Vector3::WHITE) { }
	~PixelShaderTexture() {}
};
