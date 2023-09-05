#pragma once
#include <memory>
#include <cassert>
#include "Passkey.hpp"

// TODO: add static_assert for T::UnscopedData (mb also add specialization if T not has UnscopedData)
template<typename T>
class TokenSingleton {
private:
    using token_data = typename T::UnscopedData; // UnscopedData must have default Ctor

    TokenSingleton() = default;
    static TokenSingleton<T>& GetInstance() {
        static TokenSingleton<T> instance;
        return instance;
    }

public:
    ~TokenSingleton() = default;

    static void SetToken(Passkey<T>, std::shared_ptr<int> token) {
        assert(!GetInstance().tokenWrapper && " --> token already set");
        GetInstance().tokenWrapper = new TokenWrapper(token);
    }

    static const token_data& GetData() {
        assert(GetInstance().tokenWrapper && " --> token was empty");
        return GetInstance().tokenWrapper->GetData();
    }

    static bool IsExpired() {
        assert(GetInstance().tokenWrapper && " --> token was empty");
        return GetInstance().tokenWrapper->IsExpired();
    }

private:
    class TokenWrapper {
    public:
        TokenWrapper(std::weak_ptr<int> token) : token{ token } {}
        ~TokenWrapper() = delete; // ensure at compile time that destructor not called

        bool IsExpired() {
            return token.expired();
        }

        const token_data& GetData() {
            return data;
        }

    private:
        std::weak_ptr<int> token;
        const token_data data;
    };

    TokenWrapper* tokenWrapper = nullptr;
};