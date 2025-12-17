#include "pch.h"
#include "InspectableLocks.h"
#include "LockInspector.h"

namespace thread {
    critical_section_ppl_insp::~critical_section_ppl_insp() {
        LockInspector::Instance()->OnLockDestructor(this);
    }

    mutex_std_insp::~mutex_std_insp() {
        LockInspector::Instance()->OnLockDestructor(this);
    }
}