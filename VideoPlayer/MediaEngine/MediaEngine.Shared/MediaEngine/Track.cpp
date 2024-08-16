#include "Track.h"

void Track::SetOffsetPts(H::Chrono::Hns offsetPts) {
	this->offsetPts = offsetPts;
}

H::Chrono::Hns Track::GetOffsetPts() {
	return this->offsetPts;
}