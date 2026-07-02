// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "edit_chat_notification_preferences.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ChatNotificationUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ChatNotificationUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getChatNotificationPrefs();
    Q_INVOKABLE void saveChatNotificationPrefs();

signals:
    void chatNotificationPrefsOk(EditChatNotificationPreferences*);
    void chatNotificationPrefsFailed(QString error);

private:
    bool arePreferencesModified() const;

    std::unique_ptr<EditChatNotificationPreferences> mEditChatNotificationPrefs;
};

}
