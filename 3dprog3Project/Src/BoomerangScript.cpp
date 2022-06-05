#include "pch.h"
#include "BoomerangScript.h"
#include "commonComponents.h"
#include "KeyBoardInput.h"

using namespace rfe;
using namespace rfm;

void BoomerangScript::OnStart()
{
	m_startTransform = GetComponent<TransformComp>()->transform;
}

void BoomerangScript::OnUpdate(float dt)
{
	if (KeyBoard::Get().GetKey(Key::T, KeyState::clicked))
	{

		auto [t, r, s] = rfm::decomposeToTRS(m_startTransform);
		Throw({ 0, 1, 0 }, { 0 }, { 0 }, r);
	}
}

void BoomerangScript::OnFixedUpdate(float dt)
{
	if (m_inFlight)
	{
		Transform& tr = GetComponent<TransformComp>()->transform;

		Vector3 pos = tr.getTranslation();

		pos += m_velocity * dt;
		tr.setTranslation(pos);
	}


}

void BoomerangScript::Throw(rfm::Vector3 velocity, rfm::Vector3 angularVelocity,
	rfm::Vector3 startPos, rfm::Matrix3 startRotation)
{
	m_inFlight = true;
	m_velocity = velocity;
	m_angularVelocity = angularVelocity;

	auto& tr = GetComponent<TransformComp>()->transform;


	tr.setRotation(startRotation);
	tr.setTranslation(startPos);
}
