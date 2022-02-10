#include <davinci.h>
using namespace davinci;


#define GET_ELEMENT_DEFINE(name, t) \
	inline t* _get_##name(const float* input, const VertexDesc::OffsetData& inputVertexOffset) const { \
		if(WITH_##name) \
			return (t*)(input + inputVertexOffset[(size_t)VertexElementType::VET_##name]); \
		else \
			return nullptr; \
	}

#define GET_ELEMENT(name)  _get_##name(input, inputVertexOffset)

#define SET_ELEMENT_DEFINE_BEGIN(name, t) \
	inline void _set_##name(float* vsout, const t* input, const VSConstantBuffer* param) const { \
		if(WITH_##name) { \

#define SET_ELEMENT_DEFINE_END() }}

template<bool WITH_NORMAL, bool WITH_TEXCOORD0>
class VertexShaderStandard : public VertexShader
{
public:
	struct VSConstantBuffer
	{
		Matrix4 matWorld;
		Matrix4 matViewProj;
		Vector3 eyePos;
	};

public:
	virtual void preRender(const Matrix4& worldTransform, RenderablePtr renderable) const {
		VSConstantBuffer param;
		param.matWorld = worldTransform;
		param.matViewProj = m_camera->getViewProjMatrix();
		param.eyePos = m_camera->getEye();

		renderable->setVSConstantBuffer(0, (const uint8_t*)&param, sizeof(VSConstantBuffer));
	}

	//get input elements defines
	GET_ELEMENT_DEFINE(NORMAL, Vector3)
	GET_ELEMENT_DEFINE(TEXCOORD0, Vector2)

	//Set output defines
	SET_ELEMENT_DEFINE_BEGIN(NORMAL, Vector3)
		*((Vector3*)(vsout + m_vertexOutDesc.getElementOffset(VertexElementType::VET_NORMAL))) = (Vector4((*input), 0.f) * param->matWorld).xyz().normalise();
	SET_ELEMENT_DEFINE_END()

	SET_ELEMENT_DEFINE_BEGIN(TEXCOORD0, Vector2)
		*((Vector2*)(vsout + m_vertexOutDesc.getElementOffset(VertexElementType::VET_TEXCOORD0))) = (*input);
	SET_ELEMENT_DEFINE_END()

	virtual void vsFunction(ConstConstantBufferPtr constantBuffer, const float* input, const VertexDesc::OffsetData& inputVertexOffset, float* output, float& invZ) const {
		Vector3* input_pos = (Vector3*)(input + inputVertexOffset[(size_t)VertexElementType::VET_POSITION]);
		Vector3* input_normal = GET_ELEMENT(NORMAL);
		Vector2* input_uv0 = GET_ELEMENT(TEXCOORD0);

		const VSConstantBuffer* param = (const VSConstantBuffer*)(constantBuffer->getBuffer(0));

		Vector3 worldPos = (*input_pos) * param->matWorld;

		//vsout->pos = worldPos * param->matViewProj;
		*((Vector3*)(output + m_vertexOutDesc.getElementOffset(VertexElementType::VET_POSITION))) = worldPos * param->matViewProj;
		_set_NORMAL(output, input_normal, param);
		_set_TEXCOORD0(output, input_uv0, param);

		invZ = 1.f / (worldPos - param->eyePos).length();
	}

	const VertexDesc& getOutputVertexDesc(void) const {
		return m_vertexOutDesc;
	}

private:
	VertexDesc m_vertexOutDesc;
	Camera* m_camera;

public:
	VertexShaderStandard(const VertexDesc& vsoutDesc, Camera* camera) : m_vertexOutDesc(vsoutDesc), m_camera(camera) {	}
};
