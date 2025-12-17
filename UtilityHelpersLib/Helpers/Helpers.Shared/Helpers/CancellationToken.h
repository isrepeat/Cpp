#pragma once
#include "common.h"
#include <atomic>

class CancellationToken {
public:
	CancellationToken() = default;
	~CancellationToken() = default;
	CancellationToken(CancellationToken&&) = default;
	CancellationToken& operator=(CancellationToken&& other) = default;
	
	CancellationToken(const CancellationToken&) = delete;
	CancellationToken& operator=(const CancellationToken& other) = delete;

	bool IsCanceled();
	void Cancel();
	void Reset();

private:
	std::atomic<bool> cancelled = false;
};