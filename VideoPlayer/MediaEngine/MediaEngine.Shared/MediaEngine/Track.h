#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>

class Track {
public:
	Track() = default;

	void SetOffsetPts(H::Chrono::Hns offsetPts);
	H::Chrono::Hns GetOffsetPts();

private:
	H::Chrono::Hns offsetPts = 0_hns;
};