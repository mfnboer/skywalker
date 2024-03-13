// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"

namespace Skywalker {

class PullNotifications
{
public:
    explicit PullNotifications(UserSettings& userSettings);

    void startNewMessageChecker();

private:
    bool isEnabled() const;
    bool checkPermission();
    void createNotificationChannel();

    UserSettings& mUserSettings;
};

}
