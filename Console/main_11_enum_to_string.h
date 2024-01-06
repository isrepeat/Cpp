#pragma once
#include <Windows.h>

#define CHECK(x)	\
case x:				\
Beep(800,800);		\
break;				\

void Foo(int code) {
	switch (code) {
		CHECK(1)
		CHECK(2)
		CHECK(3)
	}
}

#undef CHECK