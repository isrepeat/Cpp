#pragma once

#include <memory>
#include <tuple>

class IPack {
public:
	IPack() {}
	virtual ~IPack() {}
};

template<class ...Args>
class VarPack : public IPack {
public:
	VarPack(Args ...args)
		: pack(std::forward<Args>(args)...)
	{}

	virtual ~VarPack() {}

private:
	std::tuple<Args ...> pack;
};

template<class ...Args>
std::unique_ptr<IPack> MakeVarPack(Args ...args) {
	return std::make_unique<VarPack<Args ...>>(std::forward<Args>(args)...);
}