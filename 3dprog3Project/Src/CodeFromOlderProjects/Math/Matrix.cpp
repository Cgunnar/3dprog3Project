#include "pch.h"
#include "Matrix.hpp"
#include "Transform.hpp"
#include <DirectXMath.h>
using namespace DirectX;

namespace rfm
{
	XMMATRIX getXMMatrix(const Matrix& matrix)
	{
		return XMMATRIX((float*)(&matrix[0]));
	}

	Matrix::Matrix(Matrix3 m)
	{
		columns[0] = Vector4(m[0], 0);
		columns[1] = Vector4(m[1], 0);
		columns[2] = Vector4(m[2], 0);
		columns[3] = Vector4(0,0,0,1);
	}

	Matrix::Matrix(float* mem)
	{
		memcpy(this, mem, 64);
	}

	Matrix::Matrix(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33)
	{
		columns[0][0] = _00; columns[0][1] = _01; columns[0][2] = _02; columns[0][3] = _03;
		columns[1][0] = _10; columns[1][1] = _11; columns[1][2] = _12; columns[1][3] = _13;
		columns[2][0] = _20; columns[2][1] = _21; columns[2][2] = _22; columns[2][3] = _23;
		columns[3][0] = _30; columns[3][1] = _31; columns[3][2] = _32; columns[3][3] = _33;
	}


	Matrix::Matrix(Vector4 columnVectors[4])
	{
		columns[0] = columnVectors[0];
		columns[1] = columnVectors[1];
		columns[2] = columnVectors[2];
		columns[3] = columnVectors[3];
	}

	
	Matrix transpose(const Matrix& m)
	{
		Matrix t;
		for(int col = 0; col < 4; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				t[row][col] = m[col][row];
			}
		}
		return t;
	}
	Matrix inverse(const Matrix& matrix)
	{
		XMMATRIX inverse((float*)(&matrix[0]));
		XMFLOAT4X4 f4x4invers;
		XMStoreFloat4x4(&f4x4invers, XMMatrixInverse(nullptr, inverse));
		
		return Matrix((float*)f4x4invers.m);
	}
	std::tuple<Matrix, Matrix, Matrix> decomposeToTRS(const Matrix& matrix)
	{
#ifdef DEBUG
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				assert(!isnan(matrix[i][j]));
			}
		}
#endif // DEBUG

		

		XMVECTOR scale, rotationQuat, translation;
		XMMatrixDecompose(&scale, &rotationQuat, &translation, getXMMatrix(matrix));
		XMFLOAT4X4 S, R, T;
		XMStoreFloat4x4(&S, { XMMatrixScalingFromVector(scale) });
		XMStoreFloat4x4(&R, { XMMatrixRotationQuaternion(rotationQuat) });
		XMStoreFloat4x4(&T, { XMMatrixTranslationFromVector(translation) });
		
		return std::make_tuple(Matrix((float*)T.m), Matrix((float*)R.m), Matrix((float*)S.m));
	}
	Matrix scaleMatrix(float x, float y, float z)
	{
		Matrix s;
		s[0][0] = x;
		s[1][1] = y;
		s[2][2] = z;
		return s;
	}
	Matrix rotationFromAngles(float x, float y, float z)
	{
		return rotationZFromAngles(z) * rotationYFromAngles(y) * rotationXFromAngles(x);
	}
	Matrix rotationXFromAngles(float a)
	{
		Matrix rotX;
		rotX[1][1] = cosf(a);
		rotX[1][2] = sinf(a);
		rotX[2][1] = -sinf(a);
		rotX[2][2] = cosf(a);
		return rotX;
	}
	Matrix rotationYFromAngles(float a)
	{
		Matrix rotY;
		rotY[0][0] = cosf(a);
		rotY[0][2] = -sinf(a);
		rotY[2][0] = sinf(a);
		rotY[2][2] = cosf(a);
		return rotY;
	}
	Matrix rotationZFromAngles(float a)
	{
		Matrix rotZ;
		rotZ[0][0] = cosf(a);
		rotZ[0][1] = sinf(a);
		rotZ[1][0] = -sinf(a);
		rotZ[1][1] = cosf(a);
		return rotZ;
	}
	Matrix rotationFromAnglesDeg(float x, float y, float z)
	{
		return rotationZFromAnglesDeg(z) * rotationYFromAnglesDeg(y) * rotationXFromAnglesDeg(x);
	}
	Matrix rotationXFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationXFromAngles(a);
	}
	Matrix rotationYFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationYFromAngles(a);
	}
	Matrix rotationZFromAnglesDeg(float a)
	{
		a = XMConvertToRadians(a);
		return rotationZFromAngles(a);
	}
	Matrix rotationMatrixFromNormal(Vector3 normal, float angle)
	{
		normal.normalize();
		assert(abs(normal.length() - 1.0f) < 0.01f);
		XMVECTOR XMnormal{ normal.x, normal.y, normal.z };
		XMFLOAT4X4 XMrot;
		XMStoreFloat4x4(&XMrot, DirectX::XMMatrixRotationNormal(XMnormal, angle));

		Matrix rot = Matrix((float*)XMrot.m); //transpose???
		return rot;
	}

	Matrix PerspectiveProjectionMatrix(float FovY, float aspectRatio, float nearPlane, float farPlane)
	{
		DirectX::XMFLOAT4X4 perspectiveMatrix;
		DirectX::XMStoreFloat4x4(&perspectiveMatrix, DirectX::XMMatrixPerspectiveFovLH(FovY, aspectRatio, nearPlane, farPlane));
		return Matrix((float*)&perspectiveMatrix);
	}

	Matrix OrthographicProjectionMatrix(float width, float height, float nearPlane, float farPlane)
	{
		DirectX::XMFLOAT4X4 orthographicMatrix;
		DirectX::XMStoreFloat4x4(&orthographicMatrix, DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
		return Matrix((float*)&orthographicMatrix);
	}
	Matrix LookAt(Vector3 pos, Vector3 target, Vector3 up)
	{
		DirectX::XMFLOAT4X4 viewMatrix;
		DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixLookAtLH({ pos.x, pos.y, pos.z, 1 }, { target.x, target.y, target.z, 1 }, { up.x, up.y, up.z, 0 }));
		return Matrix((float*)&viewMatrix);
	}
	Matrix operator*(const Matrix& l, const Matrix& r)
	{
		Matrix result;
		result[0][0] = l[0][0] * r[0][0] + l[1][0] * r[0][1] + l[2][0] * r[0][2] + l[3][0] * r[0][3];
		result[0][1] = l[0][1] * r[0][0] + l[1][1] * r[0][1] + l[2][1] * r[0][2] + l[3][1] * r[0][3];
		result[0][2] = l[0][2] * r[0][0] + l[1][2] * r[0][1] + l[2][2] * r[0][2] + l[3][2] * r[0][3];
		result[0][3] = l[0][3] * r[0][0] + l[1][3] * r[0][1] + l[2][3] * r[0][2] + l[3][3] * r[0][3];

		result[1][0] = l[0][0] * r[1][0] + l[1][0] * r[1][1] + l[2][0] * r[1][2] + l[3][0] * r[1][3];
		result[1][1] = l[0][1] * r[1][0] + l[1][1] * r[1][1] + l[2][1] * r[1][2] + l[3][1] * r[1][3];
		result[1][2] = l[0][2] * r[1][0] + l[1][2] * r[1][1] + l[2][2] * r[1][2] + l[3][2] * r[1][3];
		result[1][3] = l[0][3] * r[1][0] + l[1][3] * r[1][1] + l[2][3] * r[1][2] + l[3][3] * r[1][3];

		result[2][0] = l[0][0] * r[2][0] + l[1][0] * r[2][1] + l[2][0] * r[2][2] + l[3][0] * r[2][3];
		result[2][1] = l[0][1] * r[2][0] + l[1][1] * r[2][1] + l[2][1] * r[2][2] + l[3][1] * r[2][3];
		result[2][2] = l[0][2] * r[2][0] + l[1][2] * r[2][1] + l[2][2] * r[2][2] + l[3][2] * r[2][3];
		result[2][3] = l[0][3] * r[2][0] + l[1][3] * r[2][1] + l[2][3] * r[2][2] + l[3][3] * r[2][3];

		result[3][0] = l[0][0] * r[3][0] + l[1][0] * r[3][1] + l[2][0] * r[3][2] + l[3][0] * r[3][3];
		result[3][1] = l[0][1] * r[3][0] + l[1][1] * r[3][1] + l[2][1] * r[3][2] + l[3][1] * r[3][3];
		result[3][2] = l[0][2] * r[3][0] + l[1][2] * r[3][1] + l[2][2] * r[3][2] + l[3][2] * r[3][3];
		result[3][3] = l[0][3] * r[3][0] + l[1][3] * r[3][1] + l[2][3] * r[3][2] + l[3][3] * r[3][3];
		return result;
	}

	Matrix operator+(const Matrix& l, const Matrix& r)
	{
		Matrix sum;
		sum[0] = l[0] + r[0];
		sum[1] = l[1] + r[1];
		sum[2] = l[2] + r[2];
		sum[3] = l[3] + r[3];
		return sum;
	}

	Matrix operator-(const Matrix& l, const Matrix& r)
	{
		Matrix diff;
		diff[0] = l[0] - r[0];
		diff[1] = l[1] - r[1];
		diff[2] = l[2] - r[2];
		diff[3] = l[3] - r[3];
		return diff;
	}
	

	

	Vector4& Matrix::operator[](int index) noexcept
	{
		return columns[index];
	}

	const Vector4& Matrix::operator[](int index) const noexcept
	{
		return columns[index];
	}


	//matrix vector operation-----------------------------------


	Vector4 operator*(const Matrix& m, const Vector4& v)
	{
		Matrix t = transpose(m);
		return Vector4(dot4(t[0], v), dot4(t[1], v), dot4(t[2], v), dot4(t[3], v));
	}

	Matrix3 operator*(const Matrix3& l, const Matrix3& r)
	{
		Matrix3 result;
		result[0][0] = l[0][0] * r[0][0] + l[1][0] * r[0][1] + l[2][0] * r[0][2];
		result[0][1] = l[0][1] * r[0][0] + l[1][1] * r[0][1] + l[2][1] * r[0][2];
		result[0][2] = l[0][2] * r[0][0] + l[1][2] * r[0][1] + l[2][2] * r[0][2];

		result[1][0] = l[0][0] * r[1][0] + l[1][0] * r[1][1] + l[2][0] * r[1][2];
		result[1][1] = l[0][1] * r[1][0] + l[1][1] * r[1][1] + l[2][1] * r[1][2];
		result[1][2] = l[0][2] * r[1][0] + l[1][2] * r[1][1] + l[2][2] * r[1][2];

		result[2][0] = l[0][0] * r[2][0] + l[1][0] * r[2][1] + l[2][0] * r[2][2];
		result[2][1] = l[0][1] * r[2][0] + l[1][1] * r[2][1] + l[2][1] * r[2][2];
		result[2][2] = l[0][2] * r[2][0] + l[1][2] * r[2][1] + l[2][2] * r[2][2];
		return result;
	}

	Matrix3 operator*(float scale, const Matrix3& m)
	{
		Matrix3 ret;
		ret[0] = scale * m[0];
		ret[1] = scale * m[1];
		ret[2] = scale * m[2];
		return ret;
	}

	Vector3 operator*(const Matrix3& m, const Vector3& v)
	{
		Matrix3 t = transpose(m);
		return Vector3(dot(t[0], v), dot(t[1], v), dot(t[2], v));
	}

	Matrix3 transpose(Matrix3 matrix)
	{
		std::swap(matrix[0][1], matrix[1][0]);
		std::swap(matrix[0][2], matrix[2][0]);
		std::swap(matrix[1][2], matrix[2][1]);
		return matrix;
	}

	Matrix3::Matrix3(Matrix m4x4)
	{
		columns[0] = Vector3(m4x4[0]);
		columns[1] = Vector3(m4x4[1]);
		columns[2] = Vector3(m4x4[2]);
	}

	Matrix3::Matrix3(float* mem)
	{
		memcpy(this, mem, 3*sizeof(Vector3));
	}

	Matrix3::Matrix3(float _00, float _01, float _02, float _10, float _11, float _12, float _20, float _21, float _22)
	{
		columns[0][0] = _00; columns[0][1] = _01; columns[0][2] = _02;
		columns[1][0] = _10; columns[1][1] = _11; columns[1][2] = _12;
		columns[2][0] = _20; columns[2][1] = _21; columns[2][2] = _22;
	}

	Matrix3::Matrix3(Vector3 columnVectors[3])
	{
		columns[0] = columnVectors[0];
		columns[1] = columnVectors[1];
		columns[2] = columnVectors[2];
	}

	Vector3& Matrix3::operator[](int index) noexcept
	{
		return columns[index];
	}

	const Vector3& Matrix3::operator[](int index) const noexcept
	{
		return columns[index];
	}

}