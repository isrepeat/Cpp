#include "pch.h"
#include "AggregateException.h"
#include <iterator>

AggregateException::AggregateException()
    : std::runtime_error("AggregateException")
{}

const std::vector<std::exception_ptr>& AggregateException::GetExceptions() const noexcept {
    return this->exceptions;
}

void AggregateException::RethrowIfCaptured() {
    if (!this->exceptions.empty()) {
        if (this->exceptions.size() == 1) {
           std::rethrow_exception(*this->exceptions.begin());
        }

        auto me = std::move(*this);
        throw me;
    }
}

void AggregateException::Append(std::vector<std::exception_ptr> exceptions) {
    this->exceptions.reserve(this->exceptions.size() + exceptions.size());
    std::move(exceptions.begin(), exceptions.end(), std::back_inserter(this->exceptions));
}

void AggregateException::CaptureCurrentException() {
    auto exception = std::current_exception();
    if (!exception) {
        return;
    }

    this->exceptions.push_back(std::move(exception));
}
