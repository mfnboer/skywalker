// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once

#include <atproto/lib/lexicon/app_bsky_notification.h>
#include <QObject>

namespace Skywalker {

class EditNotificationPref : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool push READ getPush WRITE setPush NOTIFY pushChanged FINAL)
    Q_PROPERTY(bool list READ getList WRITE setList NOTIFY listChanged FINAL)
    QML_ELEMENT

public:
    EditNotificationPref(QObject* parent = nullptr);
    explicit EditNotificationPref(const ATProto::AppBskyNotification::Preference::SharedPtr& pref, QObject* parent = nullptr);

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
    ATProto::AppBskyNotification::Preference::SharedPtr mPref;
    bool mModified = false;
};

}
