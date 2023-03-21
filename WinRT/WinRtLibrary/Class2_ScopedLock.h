#pragma once
#include <map>
#include <mutex>
#include <string>
#include <collection.h>



namespace AAA {
    public ref class Class2_RefScopedLockMtx sealed {
    internal:
        Class2_RefScopedLockMtx(std::mutex& mx)
            : lk{ mx }
        {
        }

    public:
        virtual ~Class2_RefScopedLockMtx() {
            int xx = 9;
        };

    private:
        std::unique_lock<std::mutex> lk;
    };
}


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


    class Temp {
    public:
        Temp() {
        }
        ~Temp() {
            int xx = 9;
        }
    };


    




    //public ref class Class2_ScopedLock sealed {
    //public:
    //    Class2_ScopedLock();
    //    virtual ~Class2_ScopedLock();

    //private:
    //    Temp temp;
    //};

    public ref class Class2_Progress sealed {
    public:
        Class2_Progress();
        virtual ~Class2_Progress();

        //Class2_ScopedLock^ LockScoped();
        AAA::Class2_RefScopedLockMtx^ LockScoped();

    private: 
        std::mutex mxData;
    };
}
