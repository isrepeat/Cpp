#pragma once
#include "IProgress.h"
#include "..\movable_ptr.h"

#include <utility>
#include <memory>
#include <cassert>

namespace thread {
    template<typename T>
    class ProgressRange {
    public:
        ProgressRange() = default;
        ProgressRange(IProgress<T>* progress, T start, T end)
            : start(start)
            , end(end)
            , distance(this->end - this->start)
            , progress(progress)
        {}

        explicit operator bool() const noexcept {
            return static_cast<bool>(this->progress);
        }

        // normalizedValue = 0...1
        void Report(T normalizedValue) {
            if (this->progress) {
                T progressVal = this->GetProgressVal(normalizedValue);
                this->progress->Report(progressVal);
            }
        }

        // normalizedStart = 0...1
        // normalizedEnd = 0...1
        ProgressRange MakeSubrange(T normalizedStart, T normalizedEnd) const {
            T subrangeStart = this->GetProgressVal(normalizedStart);
            T subrangeEnd = this->GetProgressVal(normalizedEnd);

            return ProgressRange(this->progress.get(), subrangeStart, subrangeEnd);
        }

    private:
        // normalizedValue = 0...1
        T GetProgressVal(T normalizedValue) const {
            T progressVal = this->start + this->distance * normalizedValue;

            // if fails check normalizedValue
            assert(progressVal >= this->start);
            assert(progressVal <= this->end);

            return progressVal;
        }

        T start = {};
        T end = {};
        T distance = {};
        // unique_ptr used for move semantics, lifetime not managed
        H::movable_ptr<IProgress<T>> progress;
    };
}
