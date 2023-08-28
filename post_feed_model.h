// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <QAbstractListModel>

namespace Skywalker {

class PostFeedModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Author = Qt::UserRole + 1,
        PostText,
        PostCreatedSecondsAgo,
        PostRepostedByName,
        PostImages,
        PostExternal,
        PostRecord,
        PostRecordWithMedia
    };

    explicit PostFeedModel(QObject* parent = nullptr);

    void setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    void addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
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
    Page::Ptr createPage(ATProto::AppBskyFeed::PostFeed&& feed) const;
    const Post& getPost(size_t index) const;

    std::vector<Page::Ptr> mFeedPages;
    size_t mRowCount = 0;
};

}
