// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class PostFeedModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Author = Qt::UserRole + 1,
        PostText,
        PostIndexedSecondsAgo,
        PostRepostedByName,
        PostImages,
        PostExternal,
        PostRecord,
        PostRecordWithMedia
    };

    explicit PostFeedModel(QObject* parent = nullptr);

    void setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    void addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    void insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QString getLastCursor() const;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        ATProto::AppBskyFeed::PostFeed mRawFeed;
        QString mCursorNextPage;
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed) const;

    std::deque<Post> mFeed;
    std::map<size_t, QString> mIndexCursorMap; // cursor to post at next index
    std::map<size_t, ATProto::AppBskyFeed::PostFeed> mIndexRawFeedMap; // last index in raw feed
};

}
