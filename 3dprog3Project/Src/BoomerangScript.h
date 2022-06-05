#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class BoomerangScript : public rfe::NativeScriptComponent<BoomerangScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
	void OnFixedUpdate(float dt);

	float liftCof = 0.06f;
	float dragCof = 0.04f;
	float mass = 1;
	float inertia = 1;
private:
	void Throw(rfm::Vector3 velocity, rfm::Vector3 angularVelocity, rfm::Vector3 startPos, rfm::Matrix3 startRotation);
	bool m_inFlight = false;
	rfm::Vector3 m_velocity;
	rfm::Vector3 m_angularVelocity;
	rfm::Vector3 m_throwStartPos;
	rfm::Transform m_startTransform;
};
