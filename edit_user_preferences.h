// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class EditUserPrefences : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    EditUserPrefences(ATProto::UserPreferences& userPrefs, const QString& email, QObject* parent = nullptr);

private:
    ATProto::UserPreferences& mUserPreferences;

    QString mEmail;
    std::optional<QDateTime> mBirthDate;

    ATProto::UserPreferences::FeedViewPref mHomeFeedPref;

    // Content filtering
    bool mAdultContent;
};

}
