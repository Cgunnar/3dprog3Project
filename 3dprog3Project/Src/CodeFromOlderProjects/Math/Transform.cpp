#include "pch.h"
#include "Transform.hpp"
#include <DirectXMath.h>

namespace rfm
{

	Transform::Transform(const Matrix& matrix)
	{
		m_matrix = matrix;
	}

	void Transform::setTranslation(float x, float y, float z)
	{
		m_matrix[3].x = x;
		m_matrix[3].y = y;
		m_matrix[3].z = z;
		assert(abs(m_matrix[3].w - 1.0f) < 0.00001f);
	}

	void Transform::setTranslation(const Vector3& position)
	{
		m_matrix[3].x = position.x;
		m_matrix[3].y = position.y;
		m_matrix[3].z = position.z;
		assert(abs(m_matrix[3].w - 1.0f) < 0.00001f);
	}

	void Transform::translateW(float x, float y, float z)
	{
		m_matrix[3].x += x;
		m_matrix[3].y += y;
		m_matrix[3].z += z;
	}

	void Transform::translateW(const Vector3& position)
	{
		m_matrix[3] = m_matrix[3] + position;
	}

	void Transform::translateL(float x, float y, float z)
	{
		translateL({ x, y, z });
	}

	void Transform::translateL(const Vector3& position)
	{
		translateW(this->getRotationMatrix() * Vector3(position));
	}

	void Transform::setRotation(float x, float y, float z)
	{
		setRotation(rotationFromAngles(x, y, z));
	}

	void Transform::setRotation(const Vector3& rotation)
	{
		setRotation(rotationFromAngles(rotation.x, rotation.y, rotation.z));
	}

	void Transform::setRotation(const Matrix& rotationMatrix)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		Matrix newMatrix = T * rotationMatrix * S;
		m_matrix = newMatrix;
	}

	void Transform::setRotationDeg(float x, float y, float z)
	{
		setRotation(DirectX::XMConvertToRadians(x), DirectX::XMConvertToRadians(y), DirectX::XMConvertToRadians(z));
	}

	void Transform::rotateW(float x, float y, float z)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		R = rotationFromAngles(x, y, z) * R;

		m_matrix = T * R * S;
	}

	void Transform::rotateL(float x, float y, float z)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		R = R * rotationFromAngles(x, y, z);

		m_matrix = T * R * S;
	}

	void Transform::rotateW(const Matrix& rotationMatrix)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		R = rotationMatrix * R;

		m_matrix = T * R * S;
	}

	void Transform::rotateDegW(float x, float y, float z)
	{
		rotateW(DirectX::XMConvertToRadians(x), DirectX::XMConvertToRadians(y), DirectX::XMConvertToRadians(z));
	}

	void Transform::rotateDegL(float x, float y, float z)
	{
		rotateL(DirectX::XMConvertToRadians(x), DirectX::XMConvertToRadians(y), DirectX::XMConvertToRadians(z));
	}

	void Transform::setScale(float x, float y, float z)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		S = scaleMatrix(x, y, z);
		m_matrix = T * R * S;
	}
	void Transform::setScale(const Vector3& scale)
	{
		setScale(scale.x, scale.y, scale.z);
	}
	void Transform::setScale(float scale)
	{
		this->setScale(scale, scale, scale);
	}
	void Transform::scale(float x, float y, float z)
	{
		auto [T, R, S] = decomposeToTRS(m_matrix);
		S = S * scaleMatrix(x, y, z);
		m_matrix = T * R * S;
	}
	void Transform::scale(float scale)
	{
		this->scale(scale, scale, scale);
	}
	Vector3 Transform::getTranslation() const
	{
		return m_matrix[3];
	}
	Vector3 Transform::getScale() const
	{
		Vector3 scale;
		auto S = std::get<2>(decomposeToTRS(m_matrix));
		scale.x = S[0][0];
		scale.y = S[1][1];
		scale.z = S[2][2];
		return scale;
	}
	Matrix3 Transform::getRotationMatrix() const
	{
		return std::get<1>(decomposeToTRS(m_matrix));
	}
	Vector3 Transform::forward() const
	{
		Vector3 v = m_matrix[2];
		v.normalize();
		return v;
	}
	Vector3 Transform::up() const
	{
		Vector3 v = m_matrix[1];
		v.normalize();
		return v;
	}
	Vector3 Transform::right() const
	{
		Vector3 v = m_matrix[0];
		v.normalize();
		return v;
	}
}