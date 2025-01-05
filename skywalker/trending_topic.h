// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
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
    QML_VALUE_TYPE(trendingtopic)

public:
    using List = QList<TrendingTopic>;

    TrendingTopic() = default;
    explicit TrendingTopic(const ATProto::AppBskyUnspecced::TrendingTopic::SharedPtr& topic);

    QString getTopic() const;
    QString getLink() const;

    // NormalizedWordIndex
    QString getText() const override { return getTopic(); }
    std::vector<QString> getHashtags() const override { return {}; }

private:
    ATProto::AppBskyUnspecced::TrendingTopic::SharedPtr mTopic;
};

}
