#include "pch.h"
#include "Comparators.h"

bool XMUINT2Less::operator()(const DirectX::XMUINT2 &a, const DirectX::XMUINT2 &b) const {
	if (a.y == b.y) {
		return a.x < b.x;
	}
	else {
		return a.y < b.y;
	}
}

bool XMUINT3Less::operator()(const DirectX::XMUINT3 &a, const DirectX::XMUINT3 &b) const {
	if (a.z == b.z) {
		if (a.y == b.y) {
			return a.x < b.x;
		}
		else {
			return a.y < b.y;
		}
	}
	else {
		return a.z < b.z;
	}
}

bool XMUINT3Equal::operator()(const DirectX::XMUINT3 &a, const DirectX::XMUINT3 &b) const {
	bool equ =
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z;

	return equ;
}