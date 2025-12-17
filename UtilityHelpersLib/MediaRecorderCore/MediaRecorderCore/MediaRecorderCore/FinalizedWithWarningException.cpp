#include "FinalizedWithWarningException.h"

FinalizedWithWarningException::FinalizedWithWarningException(AggregateException&& aggregateException)
    : AggregateException(std::move(aggregateException))
{}
