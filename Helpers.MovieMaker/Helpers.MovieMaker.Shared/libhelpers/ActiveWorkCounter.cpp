#include "pch.h"
#include "ActiveWorkCounter.h"

ActiveWorkCounter::IncScope::IncScope(ActiveWorkCounter *parent)
    : parent(parent)
{}

ActiveWorkCounter::IncScope::IncScope(IncScope &&other)
    : parent(other.parent)
{
    other.parent = nullptr;
}

ActiveWorkCounter::IncScope::~IncScope() {
    if (this->parent) {
        this->parent->Dec();
    }
}

ActiveWorkCounter::IncScope &ActiveWorkCounter::IncScope::operator=(IncScope &&other) {
    if (this != &other) {
        this->parent = other.parent;
        other.parent = nullptr;
    }

    return *this;
}

ActiveWorkCounter::ActiveWorkCounter()
    : workCount(0), onNoWorkAutoReset(false)
{}

ActiveWorkCounter::IncScope ActiveWorkCounter::IncScoped() {
    thread::critical_section::scoped_lock lk(this->cs);
    this->workCount++;

    return ActiveWorkCounter::IncScope(this);
}

void ActiveWorkCounter::OnNoWork(std::function<void()> onNoWork, bool autoReset) {
    thread::critical_section::scoped_lock lk(this->cs);

    if (this->workCount == 0) {
        onNoWork();
    }
    else {
        this->onNoWork = onNoWork;
        this->onNoWorkAutoReset = autoReset;
    }
}

void ActiveWorkCounter::Dec() {
    thread::critical_section::scoped_lock lk(this->cs);
    this->workCount--;

    if (this->workCount == 0 && this->onNoWork) {
        this->onNoWork();

        if (this->onNoWorkAutoReset) {
            this->onNoWork = nullptr;
            this->onNoWorkAutoReset = false;
        }
    }
}