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
    Q_PROPERTY(bool emailAuthFactor READ getEmailAuthFactor CONSTANT FINAL)
    Q_PROPERTY(QString birthDate READ getBirthDate CONSTANT FINAL)
    Q_PROPERTY(QString did READ getDID CONSTANT FINAL)
    Q_PROPERTY(QString pds READ getPDS CONSTANT FINAL)
    Q_PROPERTY(bool loggedOutVisibility READ getLoggedOutVisiblity WRITE setLoggedOutVisibility NOTIFY loggedOutVisibilityChanged FINAL)
    Q_PROPERTY(bool hideReplies READ getHideReplies WRITE setHideReplies NOTIFY hideRepliesChanged FINAL)
    Q_PROPERTY(bool hideRepliesByUnfollowed READ getHideRepliesByUnfollowed WRITE setHideRepliesByUnfollowed NOTIFY hideRepliesByUnfollowedChanged FINAL)
    Q_PROPERTY(bool hideReposts READ getHideReposts WRITE setHideReposts NOTIFY hideRepostsChanged FINAL)
    Q_PROPERTY(bool hideQuotePosts READ getHideQuotePosts WRITE setHideQuotePosts NOTIFY hideQuotePostsChanged FINAL)
    Q_PROPERTY(QEnums::AllowIncomingChat allowIncomingChat READ getAllowIncomingChat WRITE setAllowIncomingChat NOTIFY allowIncomingChatChanged FINAL)
    QML_ELEMENT

public:
    EditUserPreferences(QObject* parent = nullptr);

    void setUserPreferences(const ATProto::UserPreferences& userPreferences);
    void saveTo(ATProto::UserPreferences& userPreferences);

    const QString& getEmail() const { return mEmail; }
    void setEmail(const QString& email) { mEmail = email; }

    bool getEmailConfirmed() const { return mEmailConfirmed; }
    void setEmailConfirmed(bool confirmed) { mEmailConfirmed = confirmed; }

    bool getEmailAuthFactor() const { return mEmailAuthFactor; }
    void setEmailAuthFactor(bool set) { mEmailAuthFactor = set; }

    const QString getBirthDate() const;

    const QString& getDID() const { return mDID; }
    void setDID(const QString& did) { mDID = did; }

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

    QEnums::AllowIncomingChat getAllowIncomingChat() const { return mAllowIncomingChat; }
    void setAllowIncomingChat(QEnums::AllowIncomingChat allowIncomingChat);

signals:
    void loggedOutVisibilityChanged();
    void hideRepliesChanged();
    void hideRepliesByUnfollowedChanged();
    void hideRepostsChanged();
    void hideQuotePostsChanged();
    void allowIncomingChatChanged();

private:
    QString mEmail;
    bool mEmailConfirmed = false;
    bool mEmailAuthFactor = false;
    std::optional<QDateTime> mBirthDate;
    QString mDID;
    QString mPDS;
    bool mLoggedOutVisibility = true;

    ATProto::UserPreferences::FeedViewPref mHomeFeedPref;
    QEnums::AllowIncomingChat mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;

    // Content filtering
    bool mAdultContent = false;

    bool mModified = false;
};

}
