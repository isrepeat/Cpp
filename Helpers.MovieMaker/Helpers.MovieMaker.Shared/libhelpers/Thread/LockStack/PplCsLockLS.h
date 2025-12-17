#pragma once
#include "ILockLS.h"
#include "..\LockInspectorScope.h"
#include "..\InspectableLocks.h"

#include <ppl.h>
#include <memory>

namespace thread {
    class PplCsLock : protected ILock {
    public:
        PplCsLock(thread::critical_section_ppl_insp &cs);
        PplCsLock(std::unique_ptr<thread::critical_section_ppl_insp> &cs);
        PplCsLock(std::shared_ptr<thread::critical_section_ppl_insp> &cs);
        virtual ~PplCsLock();

    protected:
        void Lock() override;
        void Unlock() override;

    private:
        concurrency::critical_section &cs;
        LockInspectorScope inspectScope;
        concurrency::critical_section::scoped_lock lock;
    };
}