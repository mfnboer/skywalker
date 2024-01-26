// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <QHashFunctions>
#include <QString>
#include <unordered_map>

namespace Skywalker {

// The user can update his profile. Other profiles will probably never change.
// So for now, the map of changes will at most have 1 entry.
class LocalProfileChanges
{
public:
    LocalProfileChanges() = default;
    virtual ~LocalProfileChanges() = default;

    void clearLocalProfileChanges();
    const Profile* getProfileChange(const QString& did) const;
    void updateProfile(const Profile& profile);

protected:
    virtual void profileChanged() = 0;

private:
    // Mapping from DID to change
    std::unordered_map<QString, Profile> mChanges;
};

}
