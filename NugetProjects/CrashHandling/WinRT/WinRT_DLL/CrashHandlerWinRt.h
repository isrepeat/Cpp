#pragma once

namespace WinRT_DLL {
    public delegate void CrashEventCallback();

    public ref class CrashHandlerWinRt sealed {
    private:
        CrashHandlerWinRt(Platform::String^ appCenterId, Platform::String^ appVersion, Platform::String^ appUuid, Platform::String^ minidumpWriterProtocolName);
        static CrashHandlerWinRt^ instance;

    public:
        static CrashHandlerWinRt^ CreateInstance(Platform::String^ appCenterId, Platform::String^ appVersion, Platform::String^ appUuid, Platform::String^ minidumpWriterProtocolName);
        static CrashHandlerWinRt^ GetInstance();
        virtual ~CrashHandlerWinRt() {}

        event CrashEventCallback^ CrashEvent;
    };
}