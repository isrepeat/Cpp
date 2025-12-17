#pragma once
#include "Macros.h"
#include "Thread\critical_section.h"

#include <cstdint>
#include <functional>

// Used to know when wrappers use Wrapper->impl
class ActiveWorkCounter {
public:
    struct IncScope {
        NO_COPY(IncScope)

        IncScope(ActiveWorkCounter *parent);
        IncScope(IncScope &&other);
        ~IncScope();

        IncScope &operator=(IncScope &&other);

    private:
        ActiveWorkCounter *parent;
    };

    ActiveWorkCounter();

    IncScope IncScoped();

    void OnNoWork(std::function<void()> onNoWork, bool autoReset = true);

private:
    friend struct IncScope;

    thread::critical_section cs;
    uint32_t workCount;
    bool onNoWorkAutoReset;
    std::function<void()> onNoWork;

    void Dec();
};