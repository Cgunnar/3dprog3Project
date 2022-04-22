#pragma once

#include "RimfrostMath.hpp"

namespace rfm
{
	

	Vector3 closestPointOnLineFromPoint(Vector3 linePoint1, Vector3 linePoint2, Vector3 point);
	Vector3 closestPointOnLineSegmentFromPoint(Vector3 segmentEndPoint1, Vector3 segmentEndPoint2, Vector3 point);
	std::pair<Vector3, Vector3> closestPointsBetweenLines(Vector3 line1Direction, Vector3 line1Point, Vector3 line2Direction, Vector3 line2Point);
	Vector3 ProjectVectorOnPlane(Vector3 v, Plane p);


	//wrappers to directXmath
	Vector3 planeIntersectLine(Vector4 plane, Vector3 linePoint1, Vector3 linePoint2);
	Vector4 planeFromPointNormal(Vector3 point, Vector3 normal);
}