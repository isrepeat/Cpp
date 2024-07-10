/**
	\file
	\brief The Foo logic

	Detailed description about Foo logic
*/
#pragma once
#include <Helpers/Logger.h>
#include <Helpers/Time.h>

namespace SomeProject {
	/**
		\brief Main class
	*/
	class Foo {
	public:
		Foo();

		/**
			\brief Hello function

			This is the HelloTo func
		*/
		bool HelloTo(std::string to);

	private:
		void Initialize();
	};

	// Detailed description about Foo class logic
	/**
	    \class Foo
		## Main logic
		Description ...
		______

		## Drag and Drop logic
		Description ...
		______
	*/
}