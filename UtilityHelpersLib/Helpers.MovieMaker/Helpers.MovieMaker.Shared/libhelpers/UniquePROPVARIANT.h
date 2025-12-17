#pragma once

#include <PropIdl.h>

class UniquePROPVARIANT {
public:
    UniquePROPVARIANT();
    UniquePROPVARIANT(PROPVARIANT pv);
    UniquePROPVARIANT(const UniquePROPVARIANT&) = delete;
    UniquePROPVARIANT(UniquePROPVARIANT &&other);
    ~UniquePROPVARIANT();

    UniquePROPVARIANT &operator=(UniquePROPVARIANT other);
    PROPVARIANT *operator->();
    const PROPVARIANT *operator->() const;

    PROPVARIANT *get();
    const PROPVARIANT *get() const;

    void swap(UniquePROPVARIANT &other);

private:
    PROPVARIANT pv;
};