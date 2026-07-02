// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/chat_bsky_notification.h>
#include <QObject>

namespace Skywalker {

class EditChatNotificationPref : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QEnums::NotifcationFilterIncludeType includeType READ getIncludeType WRITE setIncludeType NOTIFY includeTypeChanged FINAL)
    Q_PROPERTY(bool push READ getPush WRITE setPush NOTIFY pushChanged FINAL)
    QML_ELEMENT

public:
    EditChatNotificationPref(QObject* parent = nullptr);
    explicit EditChatNotificationPref(const ATProto::ChatBskyNotification::ChatPreference::SharedPtr& pref, QObject* parent = nullptr);

    QEnums::NotifcationFilterIncludeType getIncludeType() const { return (QEnums::NotifcationFilterIncludeType)mPref->mInclude; }
    void setIncludeType(QEnums::NotifcationFilterIncludeType includeType);

    bool getPush() const { return mPref->mPush; }
    void setPush(bool push);

    bool isModified() const { return mModified; }

signals:
    void includeTypeChanged();
    void pushChanged();

private:
    ATProto::ChatBskyNotification::ChatPreference::SharedPtr mPref;
    bool mModified = false;
};

}
