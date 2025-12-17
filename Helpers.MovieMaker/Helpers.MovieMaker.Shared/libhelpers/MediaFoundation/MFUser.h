#pragma once
#include "MFInclude.h"

// This is base class for classes that will use MediaFoundation
// Ensures that MFStartup is called 1 time
class MFUser {
public:
	MFUser();

private:

	class InitSingleton {
	public:
		static InitSingleton *GetInstance();

	private:
		InitSingleton();
		InitSingleton(const InitSingleton &other) = delete;
		InitSingleton(InitSingleton &&other) = delete;

		~InitSingleton();

		InitSingleton &operator=(const InitSingleton &other) = delete;
		InitSingleton &operator=(InitSingleton &&other) = delete;
	};
};