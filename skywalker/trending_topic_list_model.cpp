// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "trending_topic_list_model.h"

namespace Skywalker {

using namespace std::chrono_literals;

TrendingTopicListModel::TrendingTopicListModel(const IMatchWords& mutedWords, QObject* parent) :
    QAbstractListModel(parent),
    mMutedWords(mutedWords)
{
}

int TrendingTopicListModel::rowCount(const QModelIndex&) const
{
    return mList.size();
}

QVariant TrendingTopicListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& topic = mList[index.row()];

    switch (Role(role))
    {
    case Role::Topic:
        return QVariant::fromValue(topic);
    case Role::TopicAgeSeconds:
        return double((QDateTime::currentDateTimeUtc() - topic.getStartedAt()) / 1s);
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void TrendingTopicListModel::clear()
{
    if (!mList.empty())
    {
        beginRemoveRows({}, 0, mList.size() - 1);
        mList.clear();
        endRemoveRows();
    }
}

void TrendingTopicListModel::addTopics(const ATProto::AppBskyUnspecced::TrendView::List& topics, int maxTopics)
{
    qDebug() << "Add topics:" << topics.size();
    std::vector<TrendingTopic> trendingTopics;

    for (const auto& topic : topics)
    {
        const TrendingTopic trendingTopic(topic);

        if (!trendingTopic.getTopic().isEmpty() && !trendingTopic.getLink().isEmpty())
        {
            if (!mMutedWords.match(trendingTopic))
                trendingTopics.push_back(trendingTopic);
        }
        else
        {
            qWarning() << "Empty topic:" << trendingTopic.getTopic() << "link:" << trendingTopic.getLink();
        }

        if ((int)trendingTopics.size() >= maxTopics)
            break;
    }

    const TrendingTopic trendingVideos(tr("Trending videos"),
                                       "https://bsky.app/profile/bsky.app/feed/thevids",
                                       tr("video"),
                                       QEnums::CONTENT_MODE_VIDEO);
    trendingTopics.push_back(trendingVideos);

    const size_t newRowCount = mList.size() + trendingTopics.size();
    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), trendingTopics.begin(), trendingTopics.end());
    endInsertRows();

    qDebug() << "New list size:" << mList.size();
}

QHash<int, QByteArray> TrendingTopicListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Topic), "topic" },
        { int(Role::TopicAgeSeconds), "topicAgeSeconds" }
    };

    return roles;
}

}
