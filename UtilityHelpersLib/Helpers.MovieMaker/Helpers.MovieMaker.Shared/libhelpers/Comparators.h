#pragma once

#include <Windows.h>
#include <DirectXMath.h>

struct XMUINT2Less {
	bool operator()(const DirectX::XMUINT2 &a, const DirectX::XMUINT2 &b) const;
};

struct XMUINT3Less {
	bool operator()(const DirectX::XMUINT3 &a, const DirectX::XMUINT3 &b) const;
};

struct XMUINT3Equal {
	bool operator()(const DirectX::XMUINT3 &a, const DirectX::XMUINT3 &b) const;
};