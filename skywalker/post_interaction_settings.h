// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class PostInteractionSettings
{
    Q_GADGET
    Q_PROPERTY(bool allowNobody READ getAllowNobody FINAL)
    Q_PROPERTY(bool allowMention READ getAllowMention FINAL)
    Q_PROPERTY(bool allowFollower READ getAllowFollower FINAL)
    Q_PROPERTY(bool allowFollowing READ getAllowFollowing FINAL)
    Q_PROPERTY(QStringList allowListUris READ getAllowListUris FINAL)
    Q_PROPERTY(bool disableEmbedding READ getDisableEmbedding FINAL)
    QML_VALUE_TYPE(postinteractionsettings)

public:
    PostInteractionSettings() = default;
    explicit PostInteractionSettings(const ATProto::AppBskyActor::PostInteractionSettingsPref& settings);

    bool getAllowNobody() const;
    bool getAllowMention() const;
    bool getAllowFollower() const;
    bool getAllowFollowing() const;
    QStringList getAllowListUris() const;
    bool getDisableEmbedding() const;

private:
    bool mAllowNobody = false;
    bool mAllowMention = false;
    bool mAllowFollower = false;
    bool mAllowFollowing = false;
    QStringList mAllowListUris;
    bool mDisableEmbedding = false;
};

}
