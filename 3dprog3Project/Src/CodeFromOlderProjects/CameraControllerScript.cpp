#include "pch.h"
#include "CameraControllerScript.h"
#include "commonComponents.h"
#include "Mouse.h"

using namespace rfm;

void CameraControllerScript::OnUpdate(float dt)
{
	if (m_lock) return;
	Mouse& in = Mouse::Get();
	MouseState ms = in.State();

	m_moveSpeed += 0.5f * in.State().deltaZ;

	m_pitch += ms.deltaY * ms.mouseCof;
	m_yaw += ms.deltaX * ms.mouseCof;
	m_pitch = std::clamp(m_pitch, -PIDIV2, PIDIV2);
	m_yaw = fmod(m_yaw, PI2);

	Vector3 moveDir{ 0,0,0 };
	if (in.State().MMBHeld)
	{
		moveDir += {0, 0, 1};
	}
	//if (in.keyBeingPressed(Input::Keys::D)) moveDir += {1, 0, 0};
	//if (in.keyBeingPressed(Input::Keys::A)) moveDir += {-1, 0, 0};
	//if (in.keyBeingPressed(Input::Keys::W)) moveDir += {0, 0, 1};
	//if (in.keyBeingPressed(Input::Keys::S)) moveDir += {0, 0, -1};


	Transform& cameraTransform = GetComponent<TransformComp>()->transform;
	cameraTransform.setRotation(m_pitch, m_yaw, 0);

	/*moveDir = cameraTransform.getRotationMatrix() * moveDir;

	if (in.keyBeingPressed(Input::Keys::Space)) moveDir += {0, 1, 0};
	if (in.keyBeingPressed(Input::Keys::C)) moveDir += {0, -1, 0};
	moveDir.normalize();

	if (in.keyBeingPressed(Input::Keys::LeftShift)) moveDir *= 2;
	moveDir *= m_moveSpeed;
	cameraTransform.translateW(moveDir * dt);*/
	cameraTransform.translateL(3*moveDir * dt);
	
}

void CameraControllerScript::ToggleCameraLock()
{
	m_lock = !m_lock;
}

bool CameraControllerScript::IsCameraLocked() const
{
	return m_lock;
}
