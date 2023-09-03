// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class BasicProfile
{
    Q_GADGET
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(QString avatarUrl READ getAvatarUrl FINAL)
    QML_VALUE_TYPE(basicprofile)

public:
    BasicProfile() = default;
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile);
    BasicProfile(const QString& handle, const QString& displayName);

    QString getName() const;
    QString getAvatarUrl() const;

private:
    const ATProto::AppBskyActor::ProfileViewBasic* mProfile = nullptr;
    QString mHandle;
    QString mDisplayName;
};

class CachedBasicProfile : public QObject
{
public:
    CachedBasicProfile() = default;
    explicit CachedBasicProfile(const BasicProfile& profile);

    const BasicProfile& getProfile() const { return mProfile; }

private:
    BasicProfile mProfile;
};

}

Q_DECLARE_METATYPE(Skywalker::BasicProfile)
