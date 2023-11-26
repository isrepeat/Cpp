#pragma once
#include <memory>
#include <cassert>

template <typename C, typename ...Args>
class SingletonScoped {
private:
    SingletonScoped() = default;
    static SingletonScoped& GetOwnInstance() {
        static SingletonScoped instance;
        return instance;
    }

public:
    ~SingletonScoped() = default;

    static C& CreateInstance(Args... args) {
        auto& singleton = SingletonScoped::GetOwnInstance();
        if (singleton.m_instance == nullptr) {
            singleton.m_instance = std::make_unique<C>(args...);
        }
        return *singleton.m_instance;
    }

    static C& GetInstance() {
        assert(SingletonScoped::GetOwnInstance().m_instance);
        return *SingletonScoped::GetOwnInstance().m_instance;
    }

private:
    std::unique_ptr<C> m_instance;
};


template <typename C, typename ...Args>
class SingletonUnscoped {
private:
    SingletonUnscoped() = default;
    static SingletonUnscoped& GetOwnInstance() {
        static SingletonUnscoped instance;
        return instance;
    }

public:
    ~SingletonUnscoped() = default;

    static C& CreateInstance(Args... args) {
        auto& singleton = SingletonUnscoped::GetOwnInstance();
        if (singleton.m_instance == nullptr) {
            singleton.m_instance = new C(args...);
        }
        return *singleton.m_instance;
    }

    static C& GetInstance() {
        assert(SingletonUnscoped::GetOwnInstance().m_instance);
        return *SingletonUnscoped::GetOwnInstance().m_instance;
    }

    static void DeleteInstance() {
        delete SingletonUnscoped::GetOwnInstance().m_instance;
        SingletonUnscoped::GetOwnInstance().m_instance = nullptr;
    }

private:
    C* m_instance = nullptr;
};