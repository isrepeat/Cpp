#pragma once

#include <mfobjects.h>
#include <functional>

class MFGenericAsyncResult {
public:
    MFGenericAsyncResult();
    MFGenericAsyncResult(std::function<void(IMFAsyncResult *)> fn);
    MFGenericAsyncResult(const MFGenericAsyncResult &other);
    MFGenericAsyncResult(MFGenericAsyncResult &&other);
    ~MFGenericAsyncResult();

    MFGenericAsyncResult &operator=(const MFGenericAsyncResult &other);
    MFGenericAsyncResult &operator=(MFGenericAsyncResult &&other);

    void CreateAsyncResult(IMFAsyncResult **res);

private:
    IMFAsyncCallback *callback;
    std::function<void(IMFAsyncResult *)> fn;
};