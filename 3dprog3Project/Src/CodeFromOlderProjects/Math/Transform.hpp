#pragma once

#include "Matrix.hpp"

namespace rfm
{
	class Transform
	{
	public:
		Transform() = default;
		~Transform() = default;

		Transform(const Matrix& matrix);

		operator Matrix& () { return m_matrix; }
		operator const Matrix& () const { return m_matrix; } 

		void setTranslation(float x, float y, float z);
		void setTranslation(const Vector3& position);
		void translateW(float x, float y, float z);
		void translateW(const Vector3& position);
		void translateL(float x, float y, float z);
		void translateL(const Vector3& position);

		void setRotation(float x, float y, float z);
		void setRotation(const Vector3& rotation);
		void setRotation(const Matrix& rotationMatrix);
		void setRotationDeg(float x, float y, float z);
		void rotateW(float x, float y, float z);
		void rotateL(float x, float y, float z);
		void rotateW(const Matrix& rotationMatrix);
		void rotateDegW(float x, float y, float z);
		void rotateDegL(float x, float y, float z);

		void setScale(float x, float y, float z);
		void setScale(const Vector3& scale);
		void setScale(float scale);
		void scale(float x, float y, float z);
		void scale(float scale);

		Vector3 getTranslation() const;
		Vector3 getScale() const;

		Matrix3 getRotationMatrix() const;

		Vector3 forward() const;
		Vector3 up() const;
		Vector3 right() const;



	private:
		Matrix m_matrix;
	};
}
