#pragma once

#include <tuple>
#include "Vector.hpp"

// Matrix[col][row]

namespace rfm
{
	class Matrix3;
	class Matrix
	{
	public:
		Matrix() = default;
		Matrix(Matrix3 m);
		Matrix(float* mem);
		Matrix(float _00, float _01, float _02, float _03,
				float _10, float _11, float _12, float _13, 
				float _20, float _21, float _22, float _23, 
				float _30, float _31, float _32, float _33);
		Matrix(Vector4 columnVectors[4]);
		~Matrix() = default;

		// Matrix[col][row]
		Vector4& operator[] (int index) noexcept;		
		const Vector4& operator[] (int index) const noexcept;		
		


	private:
		Vector4 columns[4]{ Vector4(1, 0, 0, 0),
							Vector4(0, 1, 0, 0),
							Vector4(0, 0, 1, 0),
							Vector4(0, 0, 0, 1) };
	};
	Matrix transpose(const Matrix& matrix);
	Matrix inverse(const Matrix& matrix);
	std::tuple<Matrix, Matrix, Matrix> decomposeToTRS(const Matrix& matrix);
	Matrix scaleMatrix(float x, float y, float z);
	Matrix rotationFromAngles(float x, float y, float z);
	Matrix rotationXFromAngles(float a);
	Matrix rotationYFromAngles(float a);
	Matrix rotationZFromAngles(float a);
	Matrix rotationFromAnglesDeg(float x, float y, float z);
	Matrix rotationXFromAnglesDeg(float a);
	Matrix rotationYFromAnglesDeg(float a);
	Matrix rotationZFromAnglesDeg(float a);
	Matrix rotationMatrixFromNormal(Vector3 normal, float angle);
	Matrix PerspectiveProjectionMatrix(float FovY, float aspectRatio, float nearPlane, float farPlane);
	Matrix OrthographicProjectionMatrix(float width, float height, float nearPlane, float farPlane);
	Matrix LookAt(Vector3 pos, Vector3 target, Vector3 up = { 0, 1, 0 });
	Matrix operator*(const Matrix& l, const Matrix& r);
	Matrix operator+(const Matrix& l, const Matrix& r);
	Matrix operator-(const Matrix& l, const Matrix& r);
	Vector4 operator*(const Matrix& m, const Vector4& v);



	class Matrix3
	{
	public:
		Matrix3() = default;
		Matrix3(Matrix m4x4);
		Matrix3(float* mem);
		Matrix3(float _00, float _01, float _02,
			float _10, float _11, float _12,
			float _20, float _21, float _22);
		Matrix3(Vector3 columnVectors[3]);
		~Matrix3() = default;

		// Matrix[col][row]
		Vector3& operator[] (int index) noexcept;
		const Vector3& operator[] (int index) const noexcept;



	private:
		Vector3 columns[3]{ Vector3(1, 0, 0),
							Vector3(0, 1, 0),
							Vector3(0, 0, 1)};
	};

	Matrix3 operator*(const Matrix3& l, const Matrix3& r);
	Matrix3 operator*(float scale, const Matrix3& m);
	Vector3 operator*(const Matrix3& m, const Vector3& v);
	Matrix3 transpose(Matrix3 matrix);
}