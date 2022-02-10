#include "dv_precompiled.h"
#include "dv_camera.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
void Camera::_update(void)
{
	Matrix4 matView = Matrix4::lookatLH(m_eye, m_lookat, m_up);
	Matrix4 matProj = Matrix4::perspectiveFovLH(m_fov, m_aspect, m_nearClip, m_farClip);
	m_view_proj = matView * matProj;
}

}