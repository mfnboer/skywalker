// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"

namespace Skywalker {

class IProfileMatcher
{
public:
    virtual ~IProfileMatcher() = default;
    virtual bool match(const BasicProfile&) const = 0;
};

class AnyProfileMatcher : public IProfileMatcher
{
public:
    virtual bool match(const BasicProfile&) const override { return true; }
};

class CanChatProfileMatcher : public IProfileMatcher
{
public:
    virtual bool match(const BasicProfile& profile) const override
    {
        return profile.canSendDirectMessage() && !profile.isBlocked();
    }
};

}
