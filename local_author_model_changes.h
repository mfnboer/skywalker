// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class LocalAuthorModelChanges
{
public:
    struct Change
    {
        std::optional<QString> mBlockingUri;
        std::optional<QString> mFollowingUri;
    };

    LocalAuthorModelChanges() = default;
    virtual ~LocalAuthorModelChanges() = default;

    const Change* getLocalChange(const QString& did) const;
    void clearLocalChanges();

    void updateBlockingUri(const QString& did, const QString& blockingUri);
    void updateFollowingUri(const QString& did, const QString& followingUri);

protected:
    virtual void blockingUriChanged() = 0;
    virtual void followingUriChanged() = 0;

private:
    // Mapping from author DID to change
    std::unordered_map<QString, Change> mChanges;
};

}
