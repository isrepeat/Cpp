#include <Windows.h>
#include <iostream>
#include <cassert>
#include <string>


//template <typename C, typename ...Args>
//class singleton
//{
//private:
//    singleton() {
//        int xx = 9;
//    };
//    static C* m_instance;
//
//public:
//    ~singleton()
//    {
//        delete m_instance;
//        m_instance = nullptr;
//    }
//    static C& instance(Args...args)
//    {
//        if (m_instance == nullptr)
//            m_instance = new C(args...);
//        return *m_instance;
//    }
//};
//
//template <typename C, typename ...Args>
//C* singleton<C, Args...>::m_instance = nullptr;


//class Singleton {
//private:
//    Singleton() = default;
//    static Singleton& GetInstanceOwn() {
//        static Singleton instance;
//        return instance;
//    }
//
//public:
//    ~Singleton() {
//        int xx = 9;
//        //delete m_instance;
//        //m_instance = nullptr;
//    }
//
//    static Temp& CreateInstance(float param1) {
//        auto& singleton = Singleton::GetInstanceOwn();
//        assert(!singleton.m_instance);
//        if (singleton.m_instance == nullptr) {
//            //singleton.m_instance = new Temp(param1);
//            singleton.m_instance = std::make_unique<Temp>(param1);
//        }
//        return *singleton.m_instance;
//    }
//
//    static Temp& GetInstance() {
//        auto& singleton = Singleton::GetInstanceOwn();
//        assert(singleton.m_instance);
//        return *singleton.m_instance;
//    }
//
//private:
//    //Temp* m_instance = nullptr;
//    std::unique_ptr<Temp> m_instance = nullptr;
//};


class Temp {
public:
    Temp(float param1) : data{ param1 } {
        int xxx = 9;
    }
    ~Temp() {
        int xxx = 9;
    }
private:
    float data;
};


template <typename C, typename ...Args>
class Singleton {
private:
    Singleton() = default;
    static Singleton& GetOwnInstance() {
        static Singleton instance;
        return instance;
    }

public:
    ~Singleton() = default;

    static C& CreateInstance(Args... args) {
        auto& singleton = Singleton::GetOwnInstance();
        if (singleton.m_instance == nullptr) {
            singleton.m_instance = std::make_unique<C>(args...);
        }
        return *singleton.m_instance;
    }

    static C& GetInstance() {
        assert(Singleton::GetOwnInstance().m_instance);
        return *Singleton::GetOwnInstance().m_instance;
    }

private:
    std::unique_ptr<C> m_instance = nullptr;
};

using TempSingleton = Singleton<Temp, float>;


int main() {
    Temp& a = TempSingleton::CreateInstance(3.14f);
    Temp& b = TempSingleton::GetInstance();
    Sleep(1000);
	return 0;
}
