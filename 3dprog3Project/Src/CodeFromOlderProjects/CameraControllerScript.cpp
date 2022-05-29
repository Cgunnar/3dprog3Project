#include "pch.h"
#include "CameraControllerScript.h"
#include "commonComponents.h"
#include "CodeFromOlderProjects\Mouse.h"
#include "KeyBoardInput.h"

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

	KeyBoard& kb = KeyBoard::Get();
	if (kb.GetKey(Key::D, KeyState::held)) moveDir += {1, 0, 0};
	if (kb.GetKey(Key::A, KeyState::held)) moveDir += {-1, 0, 0};
	if (kb.GetKey(Key::W, KeyState::held)) moveDir += {0, 0, 1};
	if (kb.GetKey(Key::S, KeyState::held)) moveDir += {0, 0, -1};

	Transform& cameraTransform = GetComponent<TransformComp>()->transform;
	cameraTransform.setRotation(m_pitch, m_yaw, 0);

	if (kb.GetKey(Key::space, KeyState::held)) moveDir += {0, 1, 0};
	if (kb.GetKey(Key::C, KeyState::held)) moveDir += {0, -1, 0};
	moveDir.normalize();

	if (kb.GetKey(Key::lShift, KeyState::held)) moveDir *= 2;
	moveDir *= m_moveSpeed;
	cameraTransform.translateL(moveDir * dt);
	
}

void CameraControllerScript::ToggleCameraLock()
{
	m_lock = !m_lock;
}

bool CameraControllerScript::IsCameraLocked() const
{
	return m_lock;
}
