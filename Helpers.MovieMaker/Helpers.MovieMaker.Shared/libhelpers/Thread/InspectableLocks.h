#pragma once

#include <ppl.h>
#include <mutex>

namespace thread {
    class critical_section_ppl_insp : public Concurrency::critical_section {
    public:
        ~critical_section_ppl_insp();
    };

    class mutex_std_insp : public std::mutex {
    public:
        ~mutex_std_insp();
    };
}