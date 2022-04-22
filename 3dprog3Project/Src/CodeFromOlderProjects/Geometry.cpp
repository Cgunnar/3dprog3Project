#include "pch.h"
#include "Geometry.h"
#include <DirectXMath.h>

namespace Geometry
{
	using namespace DirectX;
	using namespace rfm;
	void CalcTanAndBiTan(std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices);


	//from directXTK
	Sphere_POS_NOR_UV::Sphere_POS_NOR_UV(int tessellation, float radius)
	{
		assert(tessellation >= 3);
		int verticalSegments = tessellation;
		int horizontalSegments = tessellation * 2;

		// Create rings of vertices at progressively higher latitudes.
		for (int i = 0; i <= verticalSegments; i++)
		{
			float v = 1 - float(i) / float(verticalSegments);
			float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
			float dy, dxz;
			XMScalarSinCos(&dy, &dxz, latitude);
			// Create a single ring of vertices at this latitude.
			for (int j = 0; j <= horizontalSegments; j++)
			{
				float u = float(j) / float(horizontalSegments);
				float longitude = float(j) * XM_2PI / float(horizontalSegments);
				float dx, dz;
				XMScalarSinCos(&dx, &dz, longitude);
				dx *= dxz;
				dz *= dxz;
				rfm::Vector3 normal = {dx, dy, dz};
				rfm::Vector3 position = radius * normal;
				rfm::Vector2 textureCoordinate = { u, v };
				vertices.push_back({ position, normal, textureCoordinate });
			}
		}

		// Fill the index buffer with triangles joining each pair of latitude rings.
		int stride = horizontalSegments + 1;
		for (int i = 0; i < verticalSegments; i++)
		{
			for (int j = 0; j <= horizontalSegments; j++)
			{
				int nextI = i + 1;
				int nextJ = (j + 1) % stride;
				indices.push_back(i * stride + j);
				indices.push_back(nextI * stride + j);
				indices.push_back(i * stride + nextJ);

				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + j);
				indices.push_back(nextI * stride + nextJ);
			}
		}

		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
		for (auto& it : vertices)
		{
			it.uv.x = (1.f - it.uv.x);
		}
	}



	Sphere_POS_NOR_UV_TAN_BITAN::Sphere_POS_NOR_UV_TAN_BITAN(int tessellation, float radius)
	{
		assert(tessellation >= 3);
		int verticalSegments = tessellation;
		int horizontalSegments = tessellation * 2;

		// Create rings of vertices at progressively higher latitudes.
		for (int i = 0; i <= verticalSegments; i++)
		{
			float v = 1 - float(i) / float(verticalSegments);
			float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
			float dy, dxz;
			XMScalarSinCos(&dy, &dxz, latitude);
			// Create a single ring of vertices at this latitude.
			for (int j = 0; j <= horizontalSegments; j++)
			{
				float u = float(j) / float(horizontalSegments);
				float longitude = float(j) * XM_2PI / float(horizontalSegments);
				float dx, dz;
				XMScalarSinCos(&dx, &dz, longitude);
				dx *= dxz;
				dz *= dxz;
				rfm::Vector3 normal = { dx, dy, dz };
				/*normal.normalize();
				rfm::Vector3 tan;
				rfm::Vector3 biTan;
				CalcTanAndBiTanFromNormal(normal, tan, biTan);*/
				
				rfm::Vector3 position = radius * normal;
				rfm::Vector2 textureCoordinate = { u, v };
				//vertices.push_back({ position, normal, textureCoordinate, biTan, tan });
				vertices.push_back({ position, normal, textureCoordinate, Vector3(), Vector3() });
			}
		}

		// Fill the index buffer with triangles joining each pair of latitude rings.
		int stride = horizontalSegments + 1;
		for (int i = 0; i < verticalSegments; i++)
		{
			for (int j = 0; j <= horizontalSegments; j++)
			{
				int nextI = i + 1;
				int nextJ = (j + 1) % stride;
				indices.push_back(i * stride + j);
				indices.push_back(nextI * stride + j);
				indices.push_back(i * stride + nextJ);

				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + j);
				indices.push_back(nextI * stride + nextJ);
			}
		}

		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
		for (auto& it : vertices)
		{
			it.uv.x = (1.f - it.uv.x);
		}

		CalcTanAndBiTan(vertices, indices);


	}
	void CalcTanAndBiTan(std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices)
	{
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			auto& v0 = vertices[indices[i + 0]];
			auto& v1 = vertices[indices[i + 1]];
			auto& v2 = vertices[indices[i + 2]];

			auto dP1 = v1.position - v0.position;
			auto dP2 = v2.position - v0.position;

			auto dUV1 = v1.uv - v0.uv;
			auto dUV2 = v2.uv - v0.uv;

			/*float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
			Vector3 tangent = (dP1 * dUV2.y - dP2 * dUV1.y) * r;
			Vector3 bitangent = (dP2 * dUV1.x - dP1 * dUV2.x) * r;*/

			float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
			Vector3 tangent = (dP1 * dUV2.y - dP2 * dUV1.y) * r;
			Vector3 bitangent = -(dP2 * dUV1.x - dP1 * dUV2.x) * r;



			v0.tangent += tangent;
			v0.biTangent += bitangent;
			v1.tangent += tangent;
			v1.biTangent += bitangent;
			v2.tangent += tangent;
			v2.biTangent += bitangent;
		}
		for (auto& v : vertices)
		{
			v.biTangent.normalize();
			v.tangent.normalize();
		}
	}


	AABB_POS_NOR_UV::AABB_POS_NOR_UV(rfm::Vector3 min, rfm::Vector3 max)
	{
		Vector3 p = (min + max) / 2.0f;
		Vector3 widthHeightDepth = max - min;
		widthHeightDepth.x = abs(widthHeightDepth.x);
		widthHeightDepth.y = abs(widthHeightDepth.y);
		widthHeightDepth.z = abs(widthHeightDepth.z);
		float x = widthHeightDepth.x / 2.0f;
		float y = widthHeightDepth.y / 2.0f;
		float z = widthHeightDepth.z / 2.0f;

		   //    v6----- v5
		   //   /|      /|
		   //  v0------v3|
		   //  | |     | |
		   //  | |v7---|-|v4
		   //  |/      |/
		   //  v2------v1
		std::vector<Vertex_POS_NOR_UV> vertices
		{
			//v0 front left top
			{ p + Vector3(-x, y, -z),  Vector3(0, 0, -1),  Vector2(0, 0) },	//0
			{ p + Vector3(-x, y, -z),  Vector3(-1, 0, 0),  Vector2(1, 0) },	//1
			{ p + Vector3(-x, y, -z),  Vector3(0, 1, 0),   Vector2(0, 1) },	//2

			//v1 front right bot
			{ p + Vector3(x, -y, -z),  Vector3(0, 0, -1),  Vector2(1, 1) },	//3
			{ p + Vector3(x, -y, -z),  Vector3(1, 0, 0),   Vector2(0, 1) },	//4
			{ p + Vector3(x, -y, -z),  Vector3(0, -1, 0),  Vector2(1, 0) },	//5

			//v2 front left bot
			{ p + Vector3(-x, -y, -z), Vector3(0, 0, -1),  Vector2(0, 1) },	//6
			{ p + Vector3(-x, -y, -z), Vector3(-1, 0, 0),  Vector2(1, 1) },	//7
			{ p + Vector3(-x, -y, -z), Vector3(0, -1, 0),  Vector2(0, 0) },	//8

			//v3 front right top
			{ p + Vector3(x, y, -z),   Vector3(0, 0, -1),  Vector2(1, 0) }, //9
			{ p + Vector3(x, y, -z),   Vector3(1, 0, 0),   Vector2(0, 0) },	//10
			{ p + Vector3(x, y, -z),   Vector3(0, 1, 0),   Vector2(1, 1) },	//11

			//v4 back right bot
			{ p + Vector3(x, -y, z),   Vector3(0, 0, 1),   Vector2(0, 1) }, //12
			{ p + Vector3(x, -y, z),   Vector3(1, 0, 0),   Vector2(1, 1) },	//13
			{ p + Vector3(x, -y, z),   Vector3(0, -1, 0),  Vector2(1, 1) },	//14

			//v5 back right top
			{ p + Vector3(x, y, z),    Vector3(0, 0, 1),   Vector2(0, 0) }, //15
			{ p + Vector3(x, y, z),    Vector3(1, 0, 0),   Vector2(1, 0) },	//16
			{ p + Vector3(x, y, z),    Vector3(0, 1, 0),   Vector2(1, 0) },	//17

			//v6 back left top
			{ p + Vector3(-x, y, z),   Vector3(0, 0, 1),   Vector2(1, 0) }, //18
			{ p + Vector3(-x, y, z),   Vector3(-1, 0, 0),  Vector2(0, 0) },	//19
			{ p + Vector3(-x, y, z),   Vector3(0, 1, 0),   Vector2(0, 0) },	//20

			//v7 back left bot
			{ p + Vector3(-x, -y, z),  Vector3(0, 0, 1),   Vector2(1, 1) }, //21
			{ p + Vector3(-x, -y, z),  Vector3(-1, 0, 0),  Vector2(0, 1) },	//22
			{ p + Vector3(-x, -y, z),  Vector3(0, -1, 0),  Vector2(0, 1) },	//23
		};
		this->vertices = vertices;

		std::vector<uint32_t> indices = {
			//faces: front right back left top bot
			0, 3, 6,
			0, 9, 3,

			10, 13, 4,
			10, 16, 13,

			15, 21, 12,
			15, 18, 21,

			19, 7, 22,
			19, 1, 7,

			20, 11, 2,
			20, 17, 11,

			8, 14, 23,
			8, 5, 14
		};
		this->indices = indices;
	}
}


