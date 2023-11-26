#pragma once
#include <string>

//std::string scalarFormat(int padding, const std::string& name, const std::string& separator = "");

//std::string objectFormat(int padding, const std::string& name, const std::string& separator = "");

namespace Js {
	struct ISerialize {
		static std::string scalarFormat(int nestingLevel, const std::string& fieldName, bool endField = false);
		static std::string objectFormat(int nestingLevel, const std::string& fieldName, bool endField = false);
		static std::string arrayFormat(int nestingLevel, const std::string& fieldName, bool endField = false);

		static std::string scalarEmbeddedFromat(int nestingLevel, bool endField = false);
		static std::string objectEmbeddedFromat(int nestingLevel, bool endField = false);
		static std::string arrayEmbeddedFromat(int nestingLevel, bool endField = false);

		//static MakeFormat(std::)

		virtual std::string ToString(int nestingLevel = 1) const = 0;
	};

	void PrintJSON(ISerialize* serializableObject);
	

	// test
	void TestPrintJson();
}