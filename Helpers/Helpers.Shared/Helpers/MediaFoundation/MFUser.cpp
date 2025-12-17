#include "MFUser.h"

MFUser::MFUser() {
    InitSingleton::GetInstance();
}

MFUser::InitSingleton *MFUser::InitSingleton::GetInstance() {
    static InitSingleton singleton;
    return &singleton;
}

MFUser::InitSingleton::InitSingleton() {
    HRESULT hr = S_OK;
    hr = MFStartup(MF_VERSION);
}

MFUser::InitSingleton::~InitSingleton() {
    HRESULT hr = S_OK;
    //hr = MFShutdown();
}