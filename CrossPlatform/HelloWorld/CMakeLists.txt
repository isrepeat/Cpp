cmake_minimum_required(VERSION 3.10.0)

#project name
project (HelloWorld)

add_executable (HelloWorld
	# .h
	"src/Platform.h"
	"src/PlatformProvider.h"
	# .cpp
	"src/main.cpp"
	"src/PlatformProvider.cpp"
)