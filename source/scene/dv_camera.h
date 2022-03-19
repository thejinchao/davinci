#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class Camera
{
public:
	void setEye(const Vector3& eye, bool update=true) {
		m_eye = eye;
		if(update) _update();
	}
	const Vector3& getEye(void) const { return m_eye; }

	void setLookat(const Vector3& lookat, bool update = true) {
		m_lookat = lookat;
		if (update) _update();
	}
	const Vector3& getLookat(void) const { return m_lookat; }

	void setUp(const Vector3& up, bool update = true) {
		m_up = up;
		if (update) _update();
	}
	const Vector3& getUp(void) const { return m_up; }

	void setFov(float fov, bool update = true) {
		m_fov = fov;
		if (update) _update();
	}
	float getFov(void) const { return m_fov; }

	void setAspect(float aspect, bool update = true) {
		m_aspect = aspect;
		if (update) _update();
	}
	float getAspect(void) const {
		return m_aspect;
	}

	void setClipRange(float nearClip, float farClip, bool update = true) {
		m_nearClip = nearClip;
		m_farClip = farClip;
		if (update) _update();
	}
	float getNear(void) const { return m_nearClip; }
	float getFar(void) const { return m_farClip; }

	const Matrix4& getViewProjMatrix(void) const {
		return m_view_proj;
	}
private:
	void _update(void);

private:
	Vector3 m_eye;
	Vector3 m_lookat;
	Vector3 m_up;
	float m_fov;
	float m_aspect;
	float m_nearClip;
	float m_farClip;

	Matrix4 m_view_proj;
};

}