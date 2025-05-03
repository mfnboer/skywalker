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
    Q_PROPERTY(QString name READ getName CONSTANT FINAL)
    Q_PROPERTY(QString link MEMBER mLink CONSTANT FINAL)
    Q_PROPERTY(QString logo READ getLogo CONSTANT FINAL)
    QML_VALUE_TYPE(songlinkinfo)

public:
    using List = QList<SonglinkInfo>;

    SonglinkInfo() = default;
    SonglinkInfo(const QString& jsonKey, const QString& link);

    const QString& getName() const;
    const QString& getLogo() const;
    int getPrio() const;

private:
    QString mJsonKey;
    QString mLink;
};

class SonglinkLinks
{
    Q_GADGET
    Q_PROPERTY(SonglinkInfo::List linkInfoList MEMBER mLinkInfoList CONSTANT FINAL)
    QML_VALUE_TYPE(songlinklinks)

public:
    struct SonglinkPlatform
    {
        QString mName;
        QString mLogo;
        int mPrio;
    };

    static const std::unordered_map<QString, SonglinkPlatform> SONGLINK_PLATFORM_MAP;

    SonglinkLinks() = default;

    using SharedPtr = std::shared_ptr<SonglinkLinks>;
    static SharedPtr fromJson(const QJsonObject& json);

    Q_INVOKABLE bool isNull() const;

private:
    static QString getLink(const ATProto::XJsonObject& xjson, const QString& platform);

    SonglinkInfo::List mLinkInfoList;
};

}
