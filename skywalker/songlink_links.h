// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/xjson.h>
#include <QJsonDocument>
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class SonglinkInfo {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER mName CONSTANT FINAL)
    Q_PROPERTY(QString link MEMBER mLink CONSTANT FINAL)
    Q_PROPERTY(QString logo MEMBER mLogo CONSTANT FINAL)
    QML_VALUE_TYPE(songlinkinfo)

public:
    using List = QList<SonglinkInfo>;

    SonglinkInfo() = default;
    SonglinkInfo(const QString& name, const QString& link, const QString& logo);

private:
    QString mName;
    QString mLink;
    QString mLogo;
};

class SonglinkLinks
{
    Q_GADGET
    Q_PROPERTY(QString amazonMusic MEMBER mAmazonMusic CONSTANT FINAL)
    Q_PROPERTY(QString audioMack MEMBER mAudioMack CONSTANT FINAL)
    Q_PROPERTY(QString anghami MEMBER mAnghami CONSTANT FINAL)
    Q_PROPERTY(QString deezer MEMBER mDeezer CONSTANT FINAL)
    Q_PROPERTY(QString appleMusic MEMBER mAppleMusic CONSTANT FINAL)
    Q_PROPERTY(QString soundCloud MEMBER mSoundCloud CONSTANT FINAL)
    Q_PROPERTY(QString tidal MEMBER mTidal CONSTANT FINAL)
    Q_PROPERTY(QString youTubeMusic MEMBER mYouTubeMusic CONSTANT FINAL)
    Q_PROPERTY(QString spotify MEMBER mSpotify CONSTANT FINAL)
    QML_VALUE_TYPE(songlinklinks)

public:
    SonglinkLinks() = default;

    using SharedPtr = std::shared_ptr<SonglinkLinks>;
    static SharedPtr fromJson(const QJsonObject& json);

    Q_INVOKABLE bool isNull() const;
    Q_INVOKABLE SonglinkInfo::List getLinkInfoList() const;

private:
    static QString getLink(const ATProto::XJsonObject& xjson, const QString& platform);

    QString mAmazonMusic;
    QString mAudioMack;
    QString mAnghami;
    QString mDeezer;
    QString mAppleMusic;
    QString mSoundCloud;
    QString mTidal;
    QString mYouTubeMusic;
    QString mSpotify;
};

}
