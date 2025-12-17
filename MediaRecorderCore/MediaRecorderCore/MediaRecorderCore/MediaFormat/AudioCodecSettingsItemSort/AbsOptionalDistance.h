#pragma once

#include <optional>
#include <algorithm>

// Finds absolute(positive) distance between optional values
// Helps to compare relations(less, equals) between distances
// If some optional(a, b) has nullopt then such AbsOptionalDistance will be greater than any AbsOptionalDistance created from optionals with values
// If two AbsOptionalDistance was created with nullopt in a xor b then AbsOptionalDistance with greater a xor b value is greater
// AbsOptionalDistance range is:
//     { distance=min<T>; nullOptDistance=false } ... { distance=max<T>; nullOptDistance=false }
//     <
//     { distance=min<T>; nullOptDistance=true } ... { distance=max<T>; nullOptDistance=true }
// So AbsOptionalDistance with nullOptDistance=false is always less than nullOptDistance=true
template<class T>
class AbsOptionalDistance {
public:
    AbsOptionalDistance() = default;
    AbsOptionalDistance(const std::optional<T>& a, const std::optional<T>& b) {
        if (a && b) {
            this->nullOptDistance = false;
            this->distance = (std::max)(*a, *b) - (std::min)(*a, *b);
        }
        else if (!a && !b) {
            this->nullOptDistance = true;
            // if a and b both nullopt distance between a and b is zero
            this->distance = {};
        }
        else {
            this->nullOptDistance = true;

            if (a) {
                this->distance = *a;
            }
            else {
                this->distance = *b;
            }
        }
    }

    bool operator==(const AbsOptionalDistance& other) const {
        bool same =
            this->distance == other.distance &&
            this->nullOptDistance == other.nullOptDistance;

        return same;
    }

    bool operator<(const AbsOptionalDistance& other) const {
        if (this->nullOptDistance != other.nullOptDistance) {
            if (this->nullOptDistance) {
                // nullOptDistance is always bigger than non nullOptDistance
                return false;
            }

            return true;
        }

        return this->distance < other.distance;
    }

private:
    T distance = {};
    // true - distance between optional with value and nullopt optional
    // false - distance between optionals with values
    bool nullOptDistance = false;
};
