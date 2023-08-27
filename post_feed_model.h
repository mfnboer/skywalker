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

    void addFeed(ATProto::AppBskyFeed::PostFeed&& feed);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<Post> mFeed;
    ATProto::AppBskyFeed::PostFeed mRawFeed;
};

}
