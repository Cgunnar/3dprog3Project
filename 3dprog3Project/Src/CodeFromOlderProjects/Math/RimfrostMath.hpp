#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Transform.hpp"
#include "rfmFunctions.h"

namespace rfm
{
	struct Plane
	{
		Plane(Vector3 normal = Vector3(0, 1, 0), float d = 0);
		Plane(Vector3 normal, Vector3 point);
		Vector3 normal;
		float d;
	};

}