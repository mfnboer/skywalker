// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QObject>

namespace Skywalker
{

class RecordView
{
public:
    using Ptr = std::unique_ptr<RecordView>;

    RecordView() = default;
    explicit RecordView(const ATProto::AppBskyEmbed::RecordViewRecord* record) :
        mRecord(record)
    {}

    QString getText() const;
    BasicProfile getAuthor() const;
    QDateTime getCreatedAt() const;

    void setNotFound(bool notFound) { mNotFound = true; }
    void setBlocked(bool blocked) { mBlocked = true; }

private:
    const ATProto::AppBskyEmbed::RecordViewRecord* mRecord = nullptr;
    bool mNotFound = false;
    bool mBlocked = false;
};

}
