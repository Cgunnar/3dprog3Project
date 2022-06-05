#include "pch.h"
#include "BoomerangScript.h"
#include "commonComponents.h"
#include "KeyBoardInput.h"
#include <imgui.h>

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
		
		Throw({ 0, 10, 35 }, -15 * ((Transform)r).up(), m_startTransform.getTranslation(), r);
	}

	Transform& tr = GetComponent<TransformComp>()->transform;

	ImGui::Text(("pos: " + tr.getTranslation().ToString()).c_str());
	ImGui::Text(("v: " + m_velocity.ToString()).c_str());
	ImGui::Text(("w: " + m_angularVelocity.ToString()).c_str());






	if (m_inFlight)
	{

		if (tr.getTranslation().y < m_throwStartPos.y - 1.5f)
		{
			m_inFlight = false;
			return;
		}

		Vector3 up = tr.up();
		Vector3 velocityInForwardRightPlane = m_velocity - up * dot(m_velocity, up);

		Vector3 force;
		Vector3 torque;

		if (m_angularVelocity.length())
		{
			Vector3 w = normalize(m_angularVelocity);
			float rotationSpeed = m_angularVelocity.length() * dot(w, -up);


			//made up torque for now
			ImGui::Text(("velFR: " + velocityInForwardRightPlane.ToString()).c_str());
			Vector3 boomerangTorque = 0.05f * rotationSpeed * velocityInForwardRightPlane;
			ImGui::Text(boomerangTorque.ToString().c_str());
			torque += boomerangTorque;
			torque += -m_angularVelocity * 0.05f;

		}

		
		float dynamicPressure = 0.5f * 1.225f * m_velocity.length() * m_velocity.length();
		
		Vector3 lift = (1 - dot(normalize(m_velocity), up)) * liftCof * dynamicPressure * tr.up();
		force += lift;
		ImGui::Text(("lift: " + lift.ToString()).c_str());

		float area = 0.3f;
		Vector3 drag = dynamicPressure * area * dragCof * -normalize(m_velocity);

		force += drag;

		//gravity
		force += mass * Vector3(0, -10, 0);

		ImGui::Text(("force: " + force.ToString()).c_str());
		Vector3 acc = force / mass;
		m_velocity += acc * dt;

		ImGui::Text(("torque: " + torque.ToString()).c_str());
		Vector3 angularAcc = torque / inertia;
		m_angularVelocity += angularAcc * dt;
		

		/*if (m_velocity.length() > 200)
		{
			m_velocity *= 200 / m_velocity.length();
		}
		if (m_angularVelocity.length() > 30)
		{
			m_angularVelocity *= 20 / m_angularVelocity.length();
		}*/

		//------------------------

		Vector3 pos = tr.getTranslation();
		
		pos += m_velocity * dt;
		tr.setTranslation(pos);

		if (m_angularVelocity.length())
		{
			Vector3 w = normalize(m_angularVelocity);
			float rotationSpeed = m_angularVelocity.length() * (dot(w, -up) > 0 ? 1 : -1);
			ImGui::Text("%f", rotationSpeed);
			Matrix3 deltaRot = rotationMatrixFromNormal(w, rotationSpeed * dt);
			tr.rotateW(deltaRot);
		}
	}

}

void BoomerangScript::OnFixedUpdate(float dt)
{
	


}

void BoomerangScript::Throw(rfm::Vector3 velocity, rfm::Vector3 angularVelocity,
	rfm::Vector3 startPos, rfm::Matrix3 startRotation)
{
	m_inFlight = true;
	m_velocity = velocity;
	m_angularVelocity = angularVelocity;

	auto& tr = GetComponent<TransformComp>()->transform;

	m_throwStartPos = startPos;
	tr.setRotation(startRotation);
	tr.setTranslation(startPos);
}
