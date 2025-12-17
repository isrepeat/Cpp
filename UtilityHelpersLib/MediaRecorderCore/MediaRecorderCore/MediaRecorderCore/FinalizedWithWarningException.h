#pragma once

#include <stdexcept>
#include <libhelpers/AggregateException.h>

class FinalizedWithWarningException : public AggregateException {
public:
    using AggregateException::AggregateException;

    explicit FinalizedWithWarningException(AggregateException&& aggregateException);
};
