#pragma once

#include <d2d1.h>
#include <DirectXMath.h>
#include <vector>

struct AABBLeftTopRightBottom {};
struct AABBLeftTopSize {};

class AABB {
public:
	AABB();
	AABB(const D2D1_RECT_F &bbox);
	AABB(std::vector<DirectX::XMFLOAT2> vecPoints);
	AABB(const AABBLeftTopSize &key, DirectX::CXMVECTOR &leftTop, DirectX::CXMVECTOR &size);
	AABB(const AABBLeftTopRightBottom &key, DirectX::CXMVECTOR &leftTop, DirectX::CXMVECTOR &rightBottom);
	AABB(const AABBLeftTopSize &key, const DirectX::XMFLOAT2 &leftTop, const DirectX::XMFLOAT2 &size);
	AABB(const AABBLeftTopSize &key, float left, float top, float width, float height);
	bool DoCollide(const AABB &rect) const;
	bool Contains(const AABB& rect) const;
	
	DirectX::XMFLOAT2 Center() const { return center; }
	DirectX::XMFLOAT2 Radius() const { return radius; }
	DirectX::XMFLOAT2 Min() const { return min; }
	DirectX::XMFLOAT2 Max() const { return max; }
	float Height() const { return max.y - min.y; }
	float Width() const { return max.x - min.x; }

	bool operator==(const AABB &other) const;
	bool operator!=(const AABB &other) const;

	float GetArea() const;

private:
	DirectX::XMFLOAT2 center, radius;
	DirectX::XMFLOAT2 min, max;
};