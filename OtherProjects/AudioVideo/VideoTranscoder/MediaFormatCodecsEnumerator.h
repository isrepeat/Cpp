#pragma once
#include "MediaFormatCodecs.h"

#include <vector>
#include <guiddef.h>

class MediaFormatCodecsEnumerator {
public:
    MediaFormatCodecsEnumerator();

    //std::vector<MediaFormatCodecs> EnumerateMediaFormatCodecs() const;

private:
    template<class T>
    static std::vector<T> Intersect(const std::vector<T> &a, const std::vector<T> &b);
};