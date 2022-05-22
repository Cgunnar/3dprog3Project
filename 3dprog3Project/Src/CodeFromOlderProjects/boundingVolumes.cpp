#include "pch.h"
#include "boundingVolumes.h"
#include "RfextendedMath.hpp"
using namespace rfm;

AABB::AABB(const std::vector<rfm::Vector3>& points)
{
    min = std::numeric_limits<float>::max();
    max = std::numeric_limits<float>::min();
    for (const auto& p : points)
    {
        if (p.x < min.x) min.x = p.x;
        if (p.y < min.y) min.y = p.y;
        if (p.z < min.z) min.z = p.z;

        if (p.x > max.x) max.x = p.x;
        if (p.y > max.y) max.y = p.y;
        if (p.z > max.z) max.z = p.z;
    }
}

AABB AABB::Merge(AABB a, AABB b)
{
    if (a.max.x < b.max.x) a.max.x = b.max.x;
    if (a.max.y < b.max.y) a.max.y = b.max.y;
    if (a.max.z < b.max.z) a.max.z = b.max.z;

    if (a.min.x > b.min.x) a.min.x = b.min.x;
    if (a.min.y > b.min.y) a.min.y = b.min.y;
    if (a.min.z > b.min.z) a.min.z = b.min.z;

    return a;
}

rfm::Vector3 AABB::GetWidthHeightDepth() const 
{
    rfm::Vector3 widthHeightDepth = this->max - this->min;
    widthHeightDepth.x = abs(widthHeightDepth.x);
    widthHeightDepth.y = abs(widthHeightDepth.y);
    widthHeightDepth.z = abs(widthHeightDepth.z);
    return widthHeightDepth;
}

std::array<rfm::Vector3, 8> AABB::GetPointsTransformed(rfm::Transform m) const
{
    std::array<rfm::Vector3, 8> points;
    points[0] = min;
    points[1] = { min.x, min.y, max.z};
    points[2] = { max.x, min.y, max.z};
    points[3] = { max.x, min.y, min.z};
    points[4] = { min.x, max.y, min.z };
    points[5] = { min.x, max.y, max.z };
    points[6] = max;
    points[7] = { max.x, max.y, min.z };

    for (auto& p : points)
    {
        p = m * Vector4(p, 1);
    }

    return points;
}

//AABB operator*(rfm::Matrix m, AABB aabb)
//{
//    aabb.min = m * rfm::Vector4(aabb.min, 1);
//    aabb.max = m * rfm::Vector4(aabb.max, 1);
//    return aabb;
//}

Plane::Plane(rfm::Vector3 normal, float d)
{
    this->normal = normalize(normal);
    this->d = d;
}

Plane::Plane(Vector3 normal, Vector3 point)
{
    normal.normalize();
    this->normal = normal;
    this->d = -dot(normal, point);
}
