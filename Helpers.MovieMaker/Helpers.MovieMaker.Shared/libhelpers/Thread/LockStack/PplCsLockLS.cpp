#include "pch.h"
#include "PplCsLockLS.h"
#include "..\LockInspector.h"

namespace thread {
    PplCsLock::PplCsLock(thread::critical_section_ppl_insp &cs)
        : cs(cs), inspectScope(&cs), lock(cs)
    {
    }

    PplCsLock::PplCsLock(std::unique_ptr<thread::critical_section_ppl_insp> &cs)
        : cs(*cs), inspectScope(&cs), lock(*cs)
    {
    }

    PplCsLock::PplCsLock(std::shared_ptr<thread::critical_section_ppl_insp> &cs)
        : cs(*cs), inspectScope(&cs), lock(*cs)
    {
    }

    PplCsLock::~PplCsLock() {
    }

    void PplCsLock::Lock() {
        LockInspector::Instance()->OnLock(&this->cs);
        this->cs.lock();
    }

    void PplCsLock::Unlock() {
        this->cs.unlock();
        LockInspector::Instance()->OnUnlock(&this->cs);
    }
}