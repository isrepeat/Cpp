//#pragma once
//#include <memory>
//#include <cassert>
//#include <Helpers/Passkey.h>
//
////template <typename C, typename ...Args>
////class SingletonUnscoped {
////private:
////    SingletonUnscoped() = default;
////    static SingletonUnscoped& GetOwnInstance() {
////        static SingletonUnscoped instance;
////        return instance;
////    }
////
////public:
////    ~SingletonUnscoped() = default;
////
////    static C& CreateInstance(Args... args) {
////        auto& singleton = SingletonUnscoped::GetOwnInstance();
////        if (singleton.m_instance == nullptr) {
////            singleton.m_instance = new C(args...);
////        }
////        return *singleton.m_instance;
////    }
////
////    static C& GetInstance() {
////        assert(SingletonUnscoped::GetOwnInstance().m_instance);
////        return *SingletonUnscoped::GetOwnInstance().m_instance;
////    }
////
////    static void DeleteInstance() {
////        delete SingletonUnscoped::GetOwnInstance().m_instance;
////        SingletonUnscoped::GetOwnInstance().m_instance = nullptr;
////    }
////
////private:
////    C* m_instance = nullptr;
////};
////
////
//////template<typename Fr>
////class Token {
////    //friend Fr;
////    Token(std::weak_ptr<int> token);
////    ~Token();
////
////public:
////    bool IsExpired();
////
////private:
////	std::weak_ptr<int> token;
////};
////
////
////class Logger;
////using LoggerToken = SingletonUnscoped<Token, std::weak_ptr<int>>;
//
////template<class T>
////struct TokenWrapper {
////    using outer_class = T;
////    struct Data;
////};
//
//
////template<class T>
////struct TokenDataWrapper {
////    using data_type = T;
////    std::shared_ptr<T> token = std::make_shared<T>();
////};
//
//
//// TODO: add static_assert for T::UnscopedData
//template<typename T>
//class TokenSingleton {
//private:
//    using token_data = typename T::UnscopedData;
//
//    TokenSingleton() = default;
//    static TokenSingleton<T>& GetInstance() {
//        static TokenSingleton<T> instance;
//        return instance;
//    }
//
//public:
//    ~TokenSingleton() = default;
//
//    static bool IsExpired() {
//        assert(GetInstance().tokenWrapper && " --> token was empty");
//        return GetInstance().tokenWrapper->IsExpired();
//    }
//
//    static void SetToken(Passkey<T>, std::shared_ptr<token_data> token) {
//        assert(!GetInstance().tokenWrapper && " --> token already set");
//        GetInstance().tokenWrapper = new TokenWrapper(token);
//    }
//
//
//private:
//    class TokenWrapper {
//    public:
//        TokenWrapper(std::weak_ptr<token_data> token) : token{ token } {}
//        ~TokenWrapper() = delete;
//
//        bool IsExpired() {
//            return token.expired();
//        }
//
//    private:
//        std::weak_ptr<token_data> token;
//    };
//
//    TokenWrapper* tokenWrapper = nullptr;
//};
//
////template<typename T, typename TData>
////class TokenSingleton {
////private:
////    TokenSingleton() = default;
////    static TokenSingleton<T, TData>& GetInstance() {
////        static TokenSingleton<T, TData> instance;
////        return instance;
////    }
////
////public:
////    ~TokenSingleton() = default;
////
////    static bool IsExpired() {
////        assert(GetInstance().tokenWrapper && " --> token was empty");
////        return GetInstance().tokenWrapper->IsExpired();
////    }
////
////    void static SetToken(Passkey<T>, std::weak_ptr<TData> token) {
////        assert(!GetInstance().tokenWrapper && " --> token already set");
////        GetInstance().tokenWrapper = new TokenWrapper(token);
////    }
////
////
////private:
////    class TokenWrapper {
////    public:
////        TokenWrapper(std::weak_ptr<TData> token) : token{ token } {}
////        ~TokenWrapper() = delete;
////
////        bool IsExpired() {
////            return token.expired();
////        }
////
////    private:
////        std::weak_ptr<TData> token;
////    };
////
////    TokenWrapper* tokenWrapper = nullptr;
////};