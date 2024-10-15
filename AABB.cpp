#include "AABB.h"

AABB::AABB(const DirectX::XMFLOAT3 &m1, const DirectX::XMFLOAT3 &m2)
{
    max.x = m1.x > m2.x ? m1.x : m2.x;
    max.y = m1.y > m2.y ? m1.y : m2.y;
    max.z = m1.z > m2.z ? m1.z : m2.z;

    min.x = m1.x < m2.x ? m1.x : m2.x;
    min.y = m1.y < m2.y ? m1.y : m2.y;
    min.z = m1.z < m2.z ? m1.z : m2.z;
}

void AABB::GetPoints(std::vector<DirectX::XMFLOAT3> &points) const
{
    points.resize(8);
    points[0] = {max.x, min.y, min.z};
    points[1] = {max.x, max.y, min.z};
    points[2] = {max.x, max.y, max.z};
    points[3] = {max.x, min.y, max.z};

    points[4] = {min.x, min.y, max.z};
    points[5] = {min.x, max.y, max.z};
    points[6] = {min.x, max.y, min.z};
    points[7] = {min.x, min.y, min.z};
    return;
}
