#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class BoomerangScript : public rfe::NativeScriptComponent<BoomerangScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
	void OnFixedUpdate(float dt);
private:
	void Throw(rfm::Vector3 velocity, rfm::Vector3 angularVelocity, rfm::Vector3 startPos, rfm::Matrix3 startRotation);
	bool m_inFlight = false;
	rfm::Vector3 m_velocity;
	rfm::Vector3 m_angularVelocity;
	rfm::Transform m_startTransform;
};
