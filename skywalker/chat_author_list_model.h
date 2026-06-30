// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "chat_profile.h"
#include "content_filter.h"
#include "enums.h"
#include "local_author_model_changes.h"
#include "profile_store.h"
#include <atproto/lib/lexicon/chat_bsky_group.h>
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class ChatAuthorListModel : public QAbstractListModel,
                            public BaseListModel,
                            public LocalAuthorModelChanges
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(QString error READ getFeedError NOTIFY feedErrorChanged FINAL)

public:
    enum class Role {
        ChatAuthor = Qt::UserRole + 1,
        JoinRequestedAt,
        FollowingUri,
        BlockingUri,
        AuthorMuted,
        MutedReposts,
        HideFromTimeline,
        EndOfList
    };

    struct ListEntry
    {
        ChatBasicProfile mProfile;
        QDateTime mJoinRequestedAt; // only set for a list of join requests

        explicit ListEntry(const ChatBasicProfile& profile, QDateTime joinRequestedAt = {});
    };

    using Type = QEnums::ChatAuthorListType;
    using Ptr = std::unique_ptr<ChatAuthorListModel>;

    ChatAuthorListModel(Type type, const IProfileStore& mutedReposts,
                        const IProfileStore& timelineHide,
                        const ContentFilter& contentFilter, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    void addAuthors(ATProto::ChatBskyActor::ProfileViewBasic::List authors, const QString& cursor);
    void addJoinRequests(ATProto::ChatBskyGroup::JoinRequestView::List joinRequests, const QString& cursor);
    void prependAuthor(const ATProto::ChatBskyActor::ProfileViewBasic& author);

    void deleteAuthor(const QString& did);
    void deleteEntry(int index);

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Type getType() const { return mType; }

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

    virtual void setFeedError(const QString& error);
    void clearFeedError() { setFeedError({}); }
    const QString& getFeedError() const { return mFeedError; }

signals:
    void getFeedInProgressChanged();
    void feedErrorChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    virtual void blockingUriChanged() override;
    virtual void followingUriChanged() override;
    virtual void activitySubscriptionChanged() override {}
    virtual void mutedChanged() override;
    virtual void mutedRepostsChanged() override;
    virtual void hideFromTimelineChanged() override;

private:
    using AuthorList = std::deque<ListEntry>;

    void setEndOfList();
    AuthorList filterAuthors(const ATProto::ChatBskyActor::ProfileViewBasic::List& authors) const;
    void changeData(const QList<int>& roles) override;

    Type mType;
    const IProfileStore& mMutedReposts;
    const IProfileStore& mTimelineHide;
    const ContentFilter& mContentFilter;

    AuthorList mList;
    std::deque<ATProto::ChatBskyActor::ProfileViewBasic::List> mRawLists;
    std::deque<ATProto::ChatBskyGroup::JoinRequestView::List> mRawRequestLists;

    QString mCursor;
    bool mGetFeedInProgress = false;
    QString mFeedError;
};

}
