// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class  LocalPostModelChanges
{
public:
    struct Change
    {
        int mLikeCountDelta = 0;
        std::optional<QString> mLikeUri;
        int mReplyCountDelta = 0;
        int mRepostCountDelta = 0;
        std::optional<QString> mRepostUri;
    };

    LocalPostModelChanges() = default;

    const Change* getChange(const QString& cid) const;
    Change& getChangeForUpdate(const QString& cid);
    void clear();

private:
    // Mapping from post CID to change
    std::unordered_map<QString, Change> mChanges;
};

}
