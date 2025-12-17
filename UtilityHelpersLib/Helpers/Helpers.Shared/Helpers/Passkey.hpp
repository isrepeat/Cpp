#pragma once
#include "common.h"

template <typename T>
class Passkey {
private:
    static constexpr std::string_view templateNotes = "Primary template";

    friend T;
    Passkey() {}
    Passkey(const Passkey&) {}
    Passkey& operator=(const Passkey&) = delete;
};

template <typename T>
class Passkey<T*> {
private:
    static constexpr std::string_view templateNotes = "Specialized for <T*>";

    friend T;
    Passkey() {}
    Passkey(const Passkey&) {}
    Passkey& operator=(const Passkey&) = delete;
};