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
        explicit Entry(const Post& post);

        const Post& getPost() const { return mPost; }

    private:
        Post mPost;
    };

    PostCache();

    void clear();
    void put(const Post& post);
    const Post* get(const QString& uri) const;
    bool contains(const QString& uri) const;
    std::vector<QString> getNonCachedUris(const std::vector<QString>& uris) const;

private:
    QCache<QString, Entry> mCache; // key is at-uri
};

}
