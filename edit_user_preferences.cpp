// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "edit_user_preferences.h"

namespace Skywalker {

EditUserPrefences::EditUserPrefences(ATProto::UserPreferences& userPrefs, const QString& email, QObject* parent) :
    QObject(parent),
    mUserPreferences(userPrefs),
    mEmail(email)
{
    mBirthDate = userPrefs.getBirthDate();
    mHomeFeedPref = userPrefs.getFeedViewPref("home");
}

}
