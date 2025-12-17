#pragma once
#include "StopWatch.h"

#include <string>

class DebugScopedWatch {
public:
    DebugScopedWatch(double maxSeconds, const std::string &message);
    ~DebugScopedWatch();

    double GetCurrentSeconds() const;

private:
    StopWatch watch;
    double maxSeconds;
    std::string message;
};