// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QtAssert>
#include <memory>

namespace Skywalker {

class Presence
{
public:
    Presence() : mPresent(std::make_shared<bool>(true)) {}
    Presence(const Presence& other) : mPresent(other.mPresent), mCopy(true) {}

    ~Presence()
    {
        if (!mCopy)
        {
            Q_ASSERT(*mPresent);
            *mPresent = false;
        }
    }

    Presence& operator=(const Presence& rhs)
    {
        mPresent = rhs.mPresent;
        mCopy = true;
        return *this;
    }

    bool isPresent() const { return *mPresent; }
    operator bool() const { return isPresent(); }
    Presence getPresence() { return *this; }

private:
    std::shared_ptr<bool> mPresent;
    bool mCopy = false;
};

}
