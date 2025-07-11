// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once

#include "enums.h"
#include <atproto/lib/lexicon/app_bsky_notification.h>
#include <QObject>

namespace Skywalker {

class EditNotificationFilterablePref : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QEnums::NotifcationFilterIncludeType includeType READ getIncludeType WRITE setIncludeType NOTIFY includeTypeChanged FINAL)
    Q_PROPERTY(bool push READ getPush WRITE setPush NOTIFY pushChanged FINAL)
    Q_PROPERTY(bool list READ getList WRITE setList NOTIFY listChanged FINAL)
    QML_ELEMENT

public:
    EditNotificationFilterablePref(QObject* parent = nullptr);
    explicit EditNotificationFilterablePref(const ATProto::AppBskyNotification::FilterablePreference::SharedPtr& pref, QObject* parent = nullptr);

    QEnums::NotifcationFilterIncludeType getIncludeType() const { return (QEnums::NotifcationFilterIncludeType)mPref->mInclude; }
    void setIncludeType(QEnums::NotifcationFilterIncludeType includeType);

    bool getList() const { return mPref->mList; }
    void setList(bool list);

    bool getPush() const { return mPref->mPush; }
    void setPush(bool push);

    bool isModified() const { return mModified; }

signals:
    void includeTypeChanged();
    void listChanged();
    void pushChanged();

private:
    ATProto::AppBskyNotification::FilterablePreference::SharedPtr mPref;
    bool mModified = false;
};

}
