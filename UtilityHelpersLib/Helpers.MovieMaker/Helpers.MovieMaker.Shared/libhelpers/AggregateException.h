#pragma once

#include <vector>
#include <stdexcept>

class AggregateException : public std::runtime_error {
public:
    AggregateException();

    const std::vector<std::exception_ptr>& GetExceptions() const noexcept;

    void RethrowIfCaptured();

    void Append(std::vector<std::exception_ptr> exceptions);

    void CaptureCurrentException();

    template<class Fn>
    void CaptureException(Fn fn) {
        try {
            fn();
        }
        catch (...) {
            this->exceptions.push_back(std::current_exception());
        }
    }

private:
    std::vector<std::exception_ptr> exceptions;
};