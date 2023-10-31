#include <Windows.h>
#include <iostream>
#include <cassert>
#include <string>
#include <memory>
#include <mutex>


template <typename TClass, typename T = void*>
class _Singleton {
public:
    // Call this in constructors of singletons classes (to ensure that TClass destroy after all of them)
    static void InitSingleton() {
        GetInstance();
    }

    // Work with public Ctor without args
    static TClass& GetInstance() {
        static TClass instance;
        return instance;
    }

private:
    friend TClass;
    _Singleton() = default;
    ~_Singleton() = default;

private:
    T m_instance;
};


template <typename C>
class Singleton : public _Singleton<Singleton<C>, std::unique_ptr<C>> {
private:
    using _MyBase = _Singleton<Singleton<C>, std::unique_ptr<C>>;

public:
    using Instance_t = C&;

    Singleton() = default;
    ~Singleton() {
        int xx = 9;
    };

    template <typename ...Args>
    static Instance_t CreateInstance(Args... args) {
        auto& _this = _MyBase::GetInstance();
        std::unique_lock lk{ _this.mx };
        if (_this.m_instance == nullptr) {
            _this.m_instance = std::make_unique<C>(args...);
        }
        return *_this.m_instance;
    }

    static Instance_t GetInstance() {
        assert(_MyBase::GetInstance().m_instance);
        return *_MyBase::GetInstance().m_instance;
    }

private:
    std::mutex mx;
};



template <typename C>
class SingletonShared : public _Singleton<SingletonShared<C>, std::shared_ptr<C>> {
private:
    using _MyBase = _Singleton<SingletonShared<C>, std::shared_ptr<C>>;

public:
    using Instance_t = std::shared_ptr<C>;

    SingletonShared() = default;
    ~SingletonShared() {
        int xx = 9;
    };

    template <typename ...Args>
    static Instance_t CreateInstance(Args... args) {
        auto& _this = _MyBase::GetInstance();
        std::unique_lock lk{ _this.mx };
        if (_this.m_instance == nullptr) {
            _this.m_instance = std::make_shared<C>(args...);
        }
        return _this.m_instance;
    }

    static Instance_t GetInstance() {
        assert(_MyBase::GetInstance().m_instance);
        return _MyBase::GetInstance().m_instance;
    }

private:
    std::mutex mx;
};


template <typename C>
class SingletonUnscoped : public _Singleton<SingletonUnscoped<C>, C*> {
private:
    using _MyBase = _Singleton<SingletonUnscoped<C>, C*>;

public:
    using Instance_t = C*;

    SingletonUnscoped() = default;
    ~SingletonUnscoped() {
        int xx = 9;
    };

    template <typename ...Args>
    static Instance_t CreateInstance(Args... args) {
        auto& _this = _MyBase::GetInstance();
        std::unique_lock lk{ _this.mx };
        if (_this.m_instance == nullptr) {
            _this.m_instance = new C(args...);
        }
        return _this.m_instance;
    }

    static Instance_t GetInstance() {
        assert(_MyBase::GetInstance().m_instance);
        return _MyBase::GetInstance().m_instance;
    }

private:
    std::mutex mx;
};



class Temp {
public:
    Temp(int param1, float param2) 
        : param1{ param1 } 
        , param2{ param2 }
    {
        int xxx = 9;
    }
    ~Temp() {
        int xxx = 9;
    }

    float GetParam2() {
        return param2;
    }

private:
    int param1;
    float param2;
};


class TempS : public _Singleton<class TempS> {
private:
    using _MyBase = _Singleton<TempS>;
    friend _MyBase;

    TempS()
        : param1{ 123 }
        , param2{ 9.11f }
    {
        int xxx = 9;
    }

public:
    ~TempS() {
        int xxx = 9;
    }

    static TempS& GetInstance() {
        return _MyBase::GetInstance();
    }

    float GetParam2() {
        return param2;
    }

private:
    std::mutex ms;
    int param1;
    float param2;
};



//using TempSingleton = Singleton<Temp>;
using TempSingleton = SingletonShared<Temp>;
//using TempSingleton = SingletonUnscoped<Temp>;




class Application {
    Application() {
        TempS::InitSingleton();
        TempSingleton::InitSingleton();
    }
public:
    ~Application() {
        float xx = TempSingleton::GetInstance()->GetParam2();
        float yy = TempS::GetInstance().GetParam2();
        int zz = 9;
    }

    static Application& GetInstance() {
        static Application instance;
        return instance;
    }
};


class Application_2 {
    Application_2() {
        TempS::InitSingleton();
        TempSingleton::InitSingleton();
    }
public:
    ~Application_2() {
        float xx = TempSingleton::GetInstance()->GetParam2();
        float yy = TempS::GetInstance().GetParam2();
        int zz = 9;
    }

    static Application_2& GetInstance() {
        static Application_2 instance;
        return instance;
    }
};


int main() {
    Application::GetInstance();
    Application_2::GetInstance();

    TempSingleton::Instance_t a = TempSingleton::CreateInstance(111, 3.14f);
    TempSingleton::Instance_t b = TempSingleton::GetInstance();

    TempS& c = TempS::GetInstance();

	return 0;
}
