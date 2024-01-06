#include "FutureWrapper.h"

FutureWrapper::FutureWrapper() {
}

FutureWrapper::~FutureWrapper() {
	if (future.valid()) {

		future.wait();
	}
}