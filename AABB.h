#ifndef AABB_H
#define AABB_H

#include "DirectXMath.h"
#include <vector>

class AABB
{
  public:
    DirectX::XMFLOAT3 max;
    DirectX::XMFLOAT3 min;
    AABB() = default;
    AABB(const DirectX::XMFLOAT3 &max, const DirectX::XMFLOAT3 &min);

    void GetPoints(std::vector<DirectX::XMFLOAT3> &points) const;
};

#endif