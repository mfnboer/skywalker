// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <QCache>
#include <QObject>

namespace Skywalker {

class PostCache
{
public:
    class Entry : public QObject
    {
    public:
        Entry() = default;
        Entry(const ATProto::AppBskyFeed::PostView::SharedPtr& rawPostView, const Post& post);

        const Post& getPost() const { return mPost; }

    private:
        ATProto::AppBskyFeed::PostView::SharedPtr mRawPostView;
        Post mPost;
    };

    PostCache();

    void clear();
    void put(const ATProto::AppBskyFeed::PostView::SharedPtr& rawPostView, const Post& post);
    const Post* get(const QString& uri) const;
    bool contains(const QString& uri) const;

private:
    QCache<QString, Entry> mCache; // key is at-uri
};

}
