// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class EditUserPreferences : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString email READ getEmail CONSTANT FINAL)
    Q_PROPERTY(bool emailConfirmed READ getEmailConfirmed CONSTANT FINAL)
    Q_PROPERTY(QString birthDate READ getBirthDate CONSTANT FINAL)
    Q_PROPERTY(QString pds READ getPDS CONSTANT FINAL)
    Q_PROPERTY(bool loggedOutVisibility READ getLoggedOutVisiblity WRITE setLoggedOutVisibility NOTIFY loggedOutVisibilityChanged FINAL)
    Q_PROPERTY(bool hideReplies READ getHideReplies WRITE setHideReplies NOTIFY hideRepliesChanged FINAL)
    Q_PROPERTY(bool hideRepliesByUnfollowed READ getHideRepliesByUnfollowed WRITE setHideRepliesByUnfollowed NOTIFY hideRepliesByUnfollowedChanged FINAL)
    Q_PROPERTY(bool hideReposts READ getHideReposts WRITE setHideReposts NOTIFY hideRepostsChanged FINAL)
    Q_PROPERTY(bool hideQuotePosts READ getHideQuotePosts WRITE setHideQuotePosts NOTIFY hideQuotePostsChanged FINAL)
    Q_PROPERTY(QEnums::DisplayMode displayMode READ getDisplayMode WRITE setDisplayMode NOTIFY displayModeChanged FINAL)
    Q_PROPERTY(bool gifAutoPlay READ getGifAutoPlay WRITE setGifAutoPlay NOTIFY gifAutoPlayChanged FINAL)
    Q_PROPERTY(bool notificationsWifiOnly READ getNotificationsWifiOnly WRITE setNotificationsWifiOnly NOTIFY notificationsWifiOnlyChanged FINAL)
    QML_ELEMENT

public:
    EditUserPreferences(QObject* parent = nullptr);

    void setUserPreferences(const ATProto::UserPreferences& userPreferences);
    void saveTo(ATProto::UserPreferences& userPreferences);

    const QString& getEmail() const { return mEmail; }
    void setEmail(const QString& email) { mEmail = email; }

    bool getEmailConfirmed() const { return mEmailConfirmed; }
    void setEmailConfirmed(bool confirmed) { mEmailConfirmed = confirmed; }

    const QString getBirthDate() const;

    const QString& getPDS() const { return mPDS; }
    void setPDS(const QString& pds) { mPDS = pds; }

    bool getLoggedOutVisiblity() const { return mLoggedOutVisibility; }
    void setLoggedOutVisibility(bool visibility);

    bool getHideReplies() const { return mHomeFeedPref.mHideReplies; }
    void setHideReplies(bool hide);

    bool getHideRepliesByUnfollowed() const { return mHomeFeedPref.mHideRepliesByUnfollowed; }
    void setHideRepliesByUnfollowed(bool hide);

    bool getHideReposts() const { return mHomeFeedPref.mHideReposts; }
    void setHideReposts(bool hide);

    bool getHideQuotePosts() const { return mHomeFeedPref.mHideQuotePosts; }
    void setHideQuotePosts(bool hide);

    bool isModified() const { return mModified; }

    QEnums::DisplayMode getDisplayMode() const { return mDisplayMode; }
    void setDisplayMode(QEnums::DisplayMode displayMode);

    bool getGifAutoPlay() const { return mGifAutoPlay; }
    void setGifAutoPlay(bool autoPlay);

    bool getNotificationsWifiOnly() const { return mNotificationsWifiOnly; }
    void setNotificationsWifiOnly(bool wifiOnly);

    bool isLocalSettingsModified() const { return mLocalSettingsModified; }
    void setLocalSettingsModified(bool modified) { mLocalSettingsModified = modified; }

signals:
    void loggedOutVisibilityChanged();
    void hideRepliesChanged();
    void hideRepliesByUnfollowedChanged();
    void hideRepostsChanged();
    void hideQuotePostsChanged();
    void displayModeChanged();
    void gifAutoPlayChanged();
    void notificationsWifiOnlyChanged();

private:
    QString mEmail;
    bool mEmailConfirmed = false;
    std::optional<QDateTime> mBirthDate;
    QString mPDS;
    bool mLoggedOutVisibility = true;

    ATProto::UserPreferences::FeedViewPref mHomeFeedPref;

    // Content filtering
    bool mAdultContent = false;

    bool mModified = false;

    // Local app settings
    QEnums::DisplayMode mDisplayMode = QEnums::DISPLAY_MODE_SYSTEM;
    bool mGifAutoPlay = true;
    bool mNotificationsWifiOnly = false;
    bool mLocalSettingsModified = false;
};

}
