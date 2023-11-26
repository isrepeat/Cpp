#pragma once
#include <map>
#include <mutex>
#include <string>
#include <collection.h>


namespace WinRtLibrary
{
    //private ref class Temp sealed {
    //public:
    //    Temp() {
    //    }

    //    virtual ~Temp() {
    //        int xxx = 9;
    //    }
    //};


    //ref class singleton sealed {
    //private:
    //    singleton() {
    //        int xx = 9;
    //    };
    //    static singleton^ m_instance;

    //public:
    //    virtual ~singleton() {
    //        delete m_instance;
    //        m_instance = nullptr;
    //    }
    //    static singleton^ instance() {
    //        if (m_instance == nullptr)
    //            m_instance = ref new singleton();
    //        return m_instance;
    //    }
    //};

    //singleton^ singleton::m_instance = nullptr;


    class Temp {
    public:
        Temp(int a, float b) : _a(a), _b(b) {
            int xx = 9;
        }
        ~Temp() {
            int xx = 9;
        }

    private:
        int _a;
        float _b;
    };



    //public ref class Class3_Singleton sealed
    //{
    //public:
    //    //static property Class3_Singleton^ Instance {
    //    //    Class3_Singleton^ get() {
    //    //        static Class3_Singleton^ instance = ref new Class3_Singleton();
    //    //        return instance;
    //    //    }
    //    //}
    //    
    //    static Class3_Singleton^ GetInstance(int a, float b = 0)
    //    {
    //        static Class3_Singleton^ instance = ref new Class3_Singleton(a, b);
    //        return instance;
    //    }

    //    virtual ~Class3_Singleton() {
    //        int x = 9;
    //    }

    //private:
    //    Class3_Singleton(int a, float b) : temp(a, b) {
    //        int x = 9;
    //    }

    //    Temp temp;
    //};



    public ref class Class3_Singleton sealed
    {
    public:
        static Class3_Singleton^ CreateInstance(int a, float b) {
            if (!instance) {
                instance = ref new Class3_Singleton(a, b);
            }
            return instance;
        }

        static Class3_Singleton^ GetInstance() {
            return instance;
        }

        virtual ~Class3_Singleton() {
            int x = 9;
        }

    private:
        Class3_Singleton(int a, float b) : temp(a, b) {
            int x = 9;
        }

        Temp temp;

        static Class3_Singleton^ instance;
    };

    Class3_Singleton^ Class3_Singleton::instance = nullptr;
}
