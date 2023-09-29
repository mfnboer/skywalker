// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <unordered_map>

namespace Skywalker {

class IProfileStore
{
public:
    virtual ~IProfileStore() = default;
    virtual bool contains(const QString& did) const = 0;
    virtual const BasicProfile* get(const QString& did) const = 0;
};

class ProfileStore : public IProfileStore
{
public:
    bool contains(const QString& did) const override;
    const BasicProfile* get(const QString& did) const override;
    void add(const BasicProfile& profile);
    void remove(const QString& did);
    void clear();
    size_t size();

private:
    std::unordered_map<QString, BasicProfile> mDidProfileMap;
};

}
