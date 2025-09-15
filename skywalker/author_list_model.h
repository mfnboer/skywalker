// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "content_filter.h"
#include "enums.h"
#include "follows_activity_store.h"
#include "local_author_model_changes.h"
#include "profile.h"
#include "profile_store.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class AuthorListModel : public QAbstractListModel,
                        public BaseListModel,
                        public LocalAuthorModelChanges
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    enum class Role {
        Author = Qt::UserRole + 1,
        FollowingUri,
        BlockingUri,
        ActivitySubscription,
        ListItemUri,
        AuthorMuted,
        MutedReposts,
        HideFromTimeline,
        EndOfList
    };

    struct ListEntry
    {
        Profile mProfile;
        QString mListItemUri; // empty when not part of a list
        bool mEndOfList = false;

        explicit ListEntry(const Profile& profile, const QString& listItemUri = {});
    };

    using Type = QEnums::AuthorListType;
    using Ptr = std::unique_ptr<AuthorListModel>;

    AuthorListModel(Type type, const QString& atId, const IProfileStore& mutedReposts,
                    const IProfileStore& timelineHide,
                    const FollowsActivityStore& followsActivityStore,
                    const ContentFilter& contentFilter, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    void addAuthors(ATProto::AppBskyActor::ProfileView::List authors, const QString& cursor);
    void addAuthors(ATProto::AppBskyActor::ProfileViewDetailed::List authors, const QString& cursor);
    void addAuthors(ATProto::AppBskyGraph::ListItemView::List listItems, const QString& cursor);
    Q_INVOKABLE void prependAuthor(const Profile& author, const QString& listItemUri);
    Q_INVOKABLE void deleteEntry(int index);

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Type getType() const { return mType; }
    const QString& getAtId() const { return mAtId; }

    std::vector<QString> getActiveFollowsDids(QString& cursor) const;

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    virtual void blockingUriChanged() override;
    virtual void followingUriChanged() override;
    virtual void activitySubscriptionChanged() override;
    virtual void mutedChanged() override;
    virtual void mutedRepostsChanged() override;
    virtual void hideFromTimelineChanged() override;

private:
    using AuthorList = std::deque<ListEntry>;

    void setEndOfList();
    AuthorList filterAuthors(const ATProto::AppBskyActor::ProfileView::List& authors) const;
    void changeData(const QList<int>& roles) override;

    Type mType;
    QString mAtId;
    const IProfileStore& mMutedReposts;
    const IProfileStore& mTimelineHide;
    const FollowsActivityStore& mFollowsActivityStore;
    const ContentFilter& mContentFilter;

    AuthorList mList;
    std::deque<ATProto::AppBskyActor::ProfileView::List> mRawLists;
    std::deque<ATProto::AppBskyActor::ProfileViewDetailed::List> mRawDetailedLists;
    std::deque<ATProto::AppBskyGraph::ListItemView::List> mRawItemLists;
    std::vector<QString> mActiveFollowsDids;

    QString mCursor;
    bool mGetFeedInProgress = false;
};

}
