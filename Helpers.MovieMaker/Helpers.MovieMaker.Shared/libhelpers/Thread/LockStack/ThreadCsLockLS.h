#pragma once
#include "ILockLS.h"
#include "..\critical_section.h"

#include <memory>

namespace thread {
    // LockInspector is used inside thread::critical_section

    template<class T>
    class ThreadCsLockBase : protected ILock {
    public:
        ThreadCsLockBase(thread::critical_section &cs)
            : cs(cs), lock(cs)
        {
        }

        ThreadCsLockBase(std::unique_ptr<thread::critical_section> &cs)
            : cs(*cs), lock(cs)
        {
        }

        ThreadCsLockBase(std::shared_ptr<thread::critical_section> &cs)
            : cs(*cs), lock(cs)
        {
        }

        virtual ~ThreadCsLockBase() {
        }

    protected:
        void Lock() override {
            this->cs.lock();
        }

        void Unlock() override {
            this->cs.unlock();
        }

    private:
        thread::critical_section &cs;
        T lock;
    };

    typedef ThreadCsLockBase<thread::critical_section::scoped_lock> ThreadCsLock;
    typedef ThreadCsLockBase<thread::critical_section::scoped_yield_lock> ThreadCsYieldLock;
}