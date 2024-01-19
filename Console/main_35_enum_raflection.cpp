#include <MagicEnum/MagicEnum.h>
#include <Helpers/Flags.h>
#include <iostream>
#include <string>
#include <map>

enum EnumA {
	A1 = 0x01,
	A2 = 0x02,
	A3 = 0x04,
	A4 = 0x08,
};

void main() {
	auto resAuto = EnumA::A1 | EnumA::A3; // 5
	EnumA resEnumA_fromInt = static_cast<EnumA>(resAuto); // (A1 | A3) (5)
	EnumA resEnumA_explicitly = static_cast<EnumA>(EnumA::A1 | EnumA::A2); // (A1 | A3) (5)
	EnumA copyEnumA = resEnumA_fromInt; // (A1 | A3) (5)
	EnumA copyEnumA_ = resEnumA_explicitly; // (A1 | A3) (5)

	H::Flags<EnumA> flagsA = EnumA::A1 | EnumA::A3;
	EnumA resEnumA_from_flagsA = flagsA.ToEnum(); // (A1 | A3) (5)
	EnumA resEnumA_from_cast_flagsA = flagsA; // (A1 | A3) (5)

	if (flagsA & EnumA::A1) {
	}

	auto enumA_name = magic_enum::enum_name(resEnumA_from_cast_flagsA); // 0 (because out of bound)

	flagsA.ProcessAllFlags([](EnumA enumA) {
		switch (enumA) {
		case EnumA::A1:
			break;
		case EnumA::A2:
			break;
		case EnumA::A3:
			break;
		case EnumA::A4:
			break;
		default:
			int yyy = 0;
		}
		return false;
		});

}