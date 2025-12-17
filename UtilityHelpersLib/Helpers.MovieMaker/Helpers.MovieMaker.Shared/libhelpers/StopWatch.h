#pragma once

#include <Windows.h>
#include <chrono>
#include <cmath>

struct StopWatchPeriod {
    int64_t count;
    int64_t remainderNum;
    int64_t remainderDen;
};

class StopWatch {
    template<class... Args> struct ElapsedHelper {
        typedef void RetType;
        static RetType Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) { static_assert(false, "Not implemented"); }
        static LARGE_INTEGER Get(float time, const LARGE_INTEGER &frequency) { static_assert(false, "Not implemented"); }
    };

    template<class T, intmax_t N, intmax_t D>
    struct ElapsedHelper<std::chrono::duration<T, std::ratio<N, D>>> {
        typedef std::chrono::duration<T, std::ratio<N, D>> RetType;
        static RetType Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) {
            T durationVal = (T)(time.QuadPart * D) / (T)(frequency.QuadPart * N);
            return RetType(durationVal);
        }
        static LARGE_INTEGER Get(const RetType &time, const LARGE_INTEGER &frequency) {
            LARGE_INTEGER result;
            result.QuadPart = (time.count() * (T)frequency.QuadPart * N) / D;
            return result;
        }
    };

    template<intmax_t N, intmax_t D>
    struct ElapsedHelper<std::ratio<N, D>> {
        typedef std::chrono::duration<int64_t, std::ratio<N, D>> RetType;
        static RetType Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) {
            int64_t durationVal = (int64_t)(time.QuadPart * D) / (int64_t)(frequency.QuadPart * N);
            return RetType(durationVal);
        }
        static LARGE_INTEGER Get(const RetType &time, const LARGE_INTEGER &frequency) {
            LARGE_INTEGER result;
            result.QuadPart = (time.count() * (int64_t)frequency.QuadPart * N) / D;
            return result;
        }
    };

    template<class T, intmax_t N, intmax_t D>
    struct ElapsedHelper<T, std::ratio<N, D>> {
        typedef std::chrono::duration<T, std::ratio<N, D>> RetType;
        static RetType Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) {
            T durationVal = (T)(time.QuadPart * D) / (T)(frequency.QuadPart * N);
            return RetType(durationVal);
        }
        static LARGE_INTEGER Get(const RetType &time, const LARGE_INTEGER &frequency) {
            LARGE_INTEGER result;
            result.QuadPart = (time.count() * (T)frequency.QuadPart * N) / D;
            return result;
        }
    };

    template<>
    struct ElapsedHelper<double> {
        typedef double RetType;
        static double Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) {
            // return seconds
            double durationVal = (double)(time.QuadPart) / (double)(frequency.QuadPart);
            return durationVal;
        }
        static LARGE_INTEGER Get(RetType time, const LARGE_INTEGER &frequency) {
            LARGE_INTEGER result;
            result.QuadPart = (int64_t)std::ceil(time * (RetType)frequency.QuadPart);
            return result;
        }
    };

    template<>
    struct ElapsedHelper<float> {
        typedef float RetType;
        static float Get(const LARGE_INTEGER &time, const LARGE_INTEGER &frequency) {
            // return seconds
            float durationVal = (float)(time.QuadPart) / (float)(frequency.QuadPart);
            return durationVal;
        }
        static LARGE_INTEGER Get(RetType time, const LARGE_INTEGER &frequency) {
            LARGE_INTEGER result;
            result.QuadPart = (int64_t)std::ceilf(time * (RetType)frequency.QuadPart);
            return result;
        }
    };

public:
    StopWatch() {
        this->prevTime.QuadPart = 0;
        QueryPerformanceFrequency(&this->frequency);
    }

    float Elapsed() const {
        return this->Elapsed<float>();
    }

    template<class... Args> typename ElapsedHelper<Args...>::RetType Elapsed() const {
        LARGE_INTEGER time;

        QueryPerformanceCounter(&time);
        time.QuadPart = time.QuadPart - this->prevTime.QuadPart;

        return ElapsedHelper<Args...>::Get(time, this->frequency);
    }

    StopWatchPeriod Period(float period) {
        return this->Period(period);
    }

    template<class... Args> StopWatchPeriod Period(typename ElapsedHelper<Args...>::RetType period) {
        LARGE_INTEGER time, elapsed;
        StopWatchPeriod retult;
        auto periodTime = ElapsedHelper<Args...>::Get(period, this->frequency);

        QueryPerformanceCounter(&time);
        elapsed.QuadPart = time.QuadPart - this->prevTime.QuadPart;

        retult.count = elapsed.QuadPart / periodTime.QuadPart;
        retult.remainderNum = elapsed.QuadPart % periodTime.QuadPart;
        retult.remainderDen = periodTime.QuadPart;

        if (retult.count > 0) {
            this->prevTime.QuadPart = time.QuadPart - retult.remainderNum;
        }

        return retult;
    }

    void Start() {
        QueryPerformanceCounter(&this->prevTime);
    }

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER prevTime;
};