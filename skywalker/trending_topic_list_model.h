// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "trending_topic.h"
#include <QAbstractListModel>

namespace Skywalker {

class TrendingTopicListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Role {
        Topic = Qt::UserRole + 1
    };

    using Ptr = std::unique_ptr<TrendingTopicListModel>;

    explicit TrendingTopicListModel(const IMatchWords& mutedWords, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool empty() const { return mList.empty(); }
    void clear();
    void addTopics(const ATProto::AppBskyUnspecced::TrendingTopic::List& topics, int maxTopics);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    const IMatchWords& mMutedWords;

    using TopicList = std::vector<TrendingTopic>;
    TopicList mList;
};

}
