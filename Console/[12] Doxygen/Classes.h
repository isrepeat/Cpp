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
		\brief Foo class.

		\details This is the Foo class
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
}