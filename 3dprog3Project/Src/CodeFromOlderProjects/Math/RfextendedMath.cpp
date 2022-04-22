#include "pch.h"
#include "RfextendedMath.hpp"
#include <DirectXMath.h>

namespace rfm
{
	Vector3 closestPointOnLineFromPoint(Vector3 linePoint1, Vector3 linePoint2, Vector3 point)
	{
		Vector3 ab = linePoint1 - linePoint2;
		float t = dot(ab, point - linePoint2) / dot(ab, ab);
		return linePoint2 + t * ab;
	}

	Vector3 closestPointOnLineSegmentFromPoint(Vector3 segmentEndPoint1, Vector3 segmentEndPoint2, Vector3 point)
	{
		//closest point on linesegment to point
		//from the book Real time collision detection
		Vector3 ab = segmentEndPoint1 - segmentEndPoint2;
		float t = dot(ab, point - segmentEndPoint2) / dot(ab, ab);

		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;

		return segmentEndPoint2 + t * ab;
	}

	std::pair<Vector3, Vector3> closestPointsBetweenLines(Vector3 L1Dir, Vector3 L1P, Vector3 L2Dir, Vector3 L2P)
	{
		float L3P[3] = { L1P.x - L2P.x, L1P.y - L2P.y, L1P.z - L2P.z };

		float B[2] = { -(L1Dir.x * L3P[0] + L1Dir.y * L3P[1] + L1Dir.z * L3P[2]),
						-(L2Dir.x * L3P[0] + L2Dir.y * L3P[1] + L2Dir.z * L3P[2]) };

		float A[4] = { L1Dir.x * L1Dir.x + L1Dir.y * L1Dir.y + L1Dir.z * L1Dir.z,
						L1Dir.x * -L2Dir.x + L1Dir.y * -L2Dir.y + L1Dir.z * -L2Dir.z,
						L1Dir.x * L2Dir.x + L1Dir.y * L2Dir.y + L1Dir.z * L2Dir.z,
						L2Dir.x * -L2Dir.x + L2Dir.y * -L2Dir.y + L2Dir.z * -L2Dir.z };

		float detInv = 1.0f / (A[0] * A[3] - A[1] * A[2]);

		float x0 = detInv * A[3] * B[0] + detInv * -A[1] * B[1];
		float x1 = detInv * -A[2] * B[0] + detInv * A[0] * B[1];

		Vector3 L1CP = { L1Dir.x * x0 + L1P.x, L1Dir.y * x0 + L1P.y, L1Dir.z * x0 + L1P.z };
		Vector3 L2CP = { L2Dir.x * x1 + L2P.x, L2Dir.y * x1 + L2P.y, L2Dir.z * x1 + L2P.z };
		return std::pair<Vector3, Vector3>(L1CP, L2CP);
	}

	Vector3 ProjectVectorOnPlane(Vector3 v, Plane p)
	{
		v -= dot(v, p.normal) * p.normal;
		float d = dot(v, p.normal);
		assert(abs(d) < 0.001f);
		return v;
	}

	Vector3 planeIntersectLine(Vector4 plane, Vector3 linePoint1, Vector3 linePoint2)
	{
		DirectX::XMVECTOR XMplane{ plane.x, plane.y, plane.z, plane.w };
		DirectX::XMVECTOR XMlinePoint1{ linePoint1.x, linePoint1.y, linePoint1.z };
		DirectX::XMVECTOR XMlinePoint2{ linePoint2.x, linePoint2.y, linePoint2.z };
		DirectX::XMFLOAT3 XMresult;
		DirectX::XMStoreFloat3(&XMresult, DirectX::XMPlaneIntersectLine(XMplane, XMlinePoint1, XMlinePoint2));
		return Vector3(XMresult.x, XMresult.y, XMresult.z);
	}
	Vector4 planeFromPointNormal(Vector3 point, Vector3 normal)
	{
		DirectX::XMVECTOR XMpoint{ point.x, point.y, point.z };
		DirectX::XMVECTOR XMnormal{ normal.x, normal.y, normal.z };
		DirectX::XMFLOAT4 XMplane;
		DirectX::XMStoreFloat4(&XMplane, DirectX::XMPlaneFromPointNormal(XMpoint, XMnormal));
		return Vector4(XMplane.x, XMplane.y, XMplane.z, XMplane.w);
	}
}