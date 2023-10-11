// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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
    Q_PROPERTY(bool hideReplies READ getHideReplies WRITE setHideReplies NOTIFY hideRepliesChanged FINAL)
    Q_PROPERTY(bool hideRepliesByUnfollowed READ getHideRepliesByUnfollowed WRITE setHideRepliesByUnfollowed NOTIFY hideRepliesByUnfollowedChanged FINAL)
    Q_PROPERTY(bool hideReposts READ getHideReposts WRITE setHideReposts NOTIFY hideRepostsChanged FINAL)
    Q_PROPERTY(bool hideQuotePosts READ getHideQuotePosts WRITE setHideQuotePosts NOTIFY hideQuotePostsChanged FINAL)
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

    bool getHideReplies() const { return mHomeFeedPref.mHideReplies; }
    void setHideReplies(bool hide);

    bool getHideRepliesByUnfollowed() const { return mHomeFeedPref.mHideRepliesByUnfollowed; }
    void setHideRepliesByUnfollowed(bool hide);

    bool getHideReposts() const { return mHomeFeedPref.mHideReposts; }
    void setHideReposts(bool hide);

    bool getHideQuotePosts() const { return mHomeFeedPref.mHideQuotePosts; }
    void setHideQuotePosts(bool hide);

    bool isModified() const { return mModified; }

signals:
    void hideRepliesChanged();
    void hideRepliesByUnfollowedChanged();
    void hideRepostsChanged();
    void hideQuotePostsChanged();

private:
    QString mEmail;
    bool mEmailConfirmed;
    std::optional<QDateTime> mBirthDate;

    ATProto::UserPreferences::FeedViewPref mHomeFeedPref;

    // Content filtering
    bool mAdultContent;

    bool mModified = false;
};

}
