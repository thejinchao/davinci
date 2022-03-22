#include "dv_precompiled.h"
#include "dv_render_queue.h"

#include "device/dv_constant_buffer.h"

#include "dv_pipe_IA.h"
#include "dv_pipe_VS.h"
#include "dv_pipe_PS.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void RenderQueue::clear(void)
{
	m_queue.clear();
}

//-------------------------------------------------------------------------------------
void RenderQueue::setCamera(const Camera& camera)
{
	m_camera = camera;
}

//-------------------------------------------------------------------------------------
void RenderQueue::pushRenderable(RenderablePtr renderable)
{
	m_queue.push_back(renderable);
}

//-------------------------------------------------------------------------------------
void RenderQueue::visitorRenderable(std::function<void(ConstRenderablePtr renderable)> visitorFunc) const
{
	for (ConstRenderablePtr renderable : m_queue) {
		visitorFunc(renderable);
	}
}

//-------------------------------------------------------------------------------------
void RenderQueue::process(RenderTarget& renderTarget)
{
	//0 : Input Assember
	/*
		Renderable Queue -> IA  -> PrimitiveAfterAssember
	*/
	PrimitiveAfterAssember inputPrimitive;
	InputAssember::process(*this, inputPrimitive);



	//1 : Vertex Shader
	/*
		PrimitiveAfterAssember -> VS -> PrimitiveAfterVS
	*/
	PrimitiveAfterVS primitiveAfterVS;
	VertexShader::process(getDevice(), inputPrimitive, primitiveAfterVS);



	//2: Pixel Shader
	/*
		PrimitiveAfterVS -> PS -> Render Target Texture
	*/
	PixelShader::process(primitiveAfterVS, renderTarget);
}

}

