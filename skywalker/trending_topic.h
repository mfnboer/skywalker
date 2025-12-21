// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "normalized_word_index.h"
#include <atproto/lib/lexicon/app_bsky_unspecced.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TrendingTopic : public NormalizedWordIndex
{
    Q_GADGET
    Q_PROPERTY(QString topic READ getTopic FINAL)
    Q_PROPERTY(QString link READ getLink FINAL)
    Q_PROPERTY(QDateTime startedAt READ getStartedAt FINAL)
    Q_PROPERTY(QEnums::TrendStatus status READ getStatus FINAL)
    Q_PROPERTY(QString category READ getCategory FINAL)
    Q_PROPERTY(int postCount READ getPostCount FINAL)
    Q_PROPERTY(QEnums::ContentMode contentMode READ getContentMode FINAL)
    QML_VALUE_TYPE(trendingtopic)

public:
    using List = QList<TrendingTopic>;

    TrendingTopic() = default;
    explicit TrendingTopic(const ATProto::AppBskyUnspecced::TrendView::SharedPtr& topic);
    TrendingTopic(const QString& topic, const QString& link, const QString& category, QEnums::ContentMode contentMode);

    QString getTopic() const;
    QString getLink() const;
    QDateTime getStartedAt() const;
    QEnums::TrendStatus getStatus() const;
    QString getCategory() const;
    int getPostCount() const;
    QEnums::ContentMode getContentMode() const { return mContentMode; }

    // NormalizedWordIndex
    QString getText() const override { return getTopic(); }
    QList<ImageView> getImages() const override { return {}; }
    VideoView::Ptr getVideoView() const override { return {}; }
    ExternalView::Ptr getExternalView() const override { return {}; }
    std::vector<QString> getHashtags() const override { return {}; }
    std::vector<QString> getWebLinks() const override { return {}; }
    BasicProfile getAuthor() const override { return {}; }

private:
    ATProto::AppBskyUnspecced::TrendView::SharedPtr mTopic;
    QEnums::ContentMode mContentMode = QEnums::CONTENT_MODE_UNSPECIFIED;
};

}
