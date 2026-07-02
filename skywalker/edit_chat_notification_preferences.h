// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "edit_chat_notification_pref.h"
#include <QObject>

namespace Skywalker {

class EditChatNotificationPreferences : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EditChatNotificationPref* chat READ getChat CONSTANT FINAL)
    Q_PROPERTY(EditChatNotificationPref* chatRequest READ getChatRequest CONSTANT FINAL)
    QML_ELEMENT

public:
    EditChatNotificationPreferences(QObject* parent = nullptr);
    EditChatNotificationPreferences(const ATProto::ChatBskyNotification::Preferences::SharedPtr& prefs,
                                    QObject* parent = nullptr);

    bool isNull() const { return !mPrefs; }

    const ATProto::ChatBskyNotification::Preferences::SharedPtr& getPrefs() const { return mPrefs; }

    EditChatNotificationPref* getChat() const { return mChatPref.get(); }
    bool isChatModified() const { return mChatPref->isModified(); }

    EditChatNotificationPref* getChatRequest() const { return mChatRequestPref.get(); }
    bool isChatRequestModified() const { return mChatRequestPref->isModified(); }

private:
    ATProto::ChatBskyNotification::Preferences::SharedPtr mPrefs;

    std::unique_ptr<EditChatNotificationPref> mChatPref;
    std::unique_ptr<EditChatNotificationPref> mChatRequestPref;
};

}
