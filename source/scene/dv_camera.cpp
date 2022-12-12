#include "dv_precompiled.h"
#include "dv_camera.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void Camera::_update(void)
{
	fMatrix4 matView = fMatrix4::lookatLH(m_eye, m_lookat, m_up);
	fMatrix4 matProj = fMatrix4::perspectiveFovLH(m_fov, m_aspect, m_nearClip, m_farClip);
	m_view_proj = matView * matProj;
}

}