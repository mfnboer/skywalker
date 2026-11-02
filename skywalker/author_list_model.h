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

namespace Skywalker {

struct AuthorListEntry
{
    Profile mProfile;
    QString mListItemUri; // empty when not part of a list
    bool mOptedOut = false;
    bool mEndOfList = false;

    explicit AuthorListEntry(const Profile& profile, const QString& listItemUri = {}, bool optedOut = false);
};

class AuthorListModel : public BaseListModel<AuthorListEntry>,
                        public LocalAuthorModelChanges
{
    Q_OBJECT
    Q_PROPERTY(int countDelta READ getCountDelta NOTIFY countDeltaChanged FINAL)
    Q_PROPERTY(QEnums::AuthorListType type READ getType CONSTANT FINAL)

public:
    enum class Role {
        Author = Qt::UserRole + 1,
        FollowingUri,
        BlockingUri,
        ActivitySubscription,
        ListItemUri,
        ListItemOptedOut,
        AuthorMuted,
        MutedReposts,
        HideFromTimeline,
        EndOfList
    };

    using Type = QEnums::AuthorListType;
    using Ptr = std::unique_ptr<AuthorListModel>;

    AuthorListModel(Type type, const QString& atId,
                    const IProfileStore& timelineHide,
                    const FollowsActivityStore& followsActivityStore,
                    const ContentFilter& contentFilter,
                    const QString& userDid, QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear() override;
    int addAuthors(ATProto::AppBskyActor::ProfileView::List authors, const QString& cursor);
    int addAuthors(ATProto::AppBskyActor::ProfileViewDetailed::List authors, const QString& cursor);
    int addAuthors(ATProto::AppBskyGraph::ListItemView::List listItems, const QString& cursor);
    Q_INVOKABLE void prependAuthor(const Profile& author, const QString& listItemUri);
    Q_INVOKABLE void prependBasicProfile(const BasicProfile& author, const QString& listItemUri);
    Q_INVOKABLE void deleteEntry(int index);

    void setCursor(const QString& cursor) override;

    Type getType() const { return mType; }

    const QString& getAtId() const { return mAtId; }
    Q_INVOKABLE void setAtId(QString atId) { mAtId = atId; }

    std::vector<QString> getActiveFollowsDids(QString& cursor) const;
    Q_INVOKABLE bool containsDid(const QString& did) const;

    void setCountDelta(int delta);
    int getCountDelta() const { return mCountDelta; }

signals:
    void countDeltaChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    virtual void blockingUriChanged() override;
    virtual void followingUriChanged() override;
    virtual void activitySubscriptionChanged() override;
    virtual void mutedChanged() override;
    virtual void mutedRepostsChanged() override;
    virtual void listItemOptedOutChanged() override;
    virtual void hideFromTimelineChanged() override;

private:
    using AuthorList = EntityList;

    void setEndOfList();
    AuthorList filterAuthors(const ATProto::AppBskyActor::ProfileView::List& authors) const;
    void changeData(const QList<int>& roles) override;

    Type mType;
    QString mAtId;
    const IProfileStore& mTimelineHide;
    const FollowsActivityStore& mFollowsActivityStore;
    const ContentFilter& mContentFilter;

    std::unordered_set<QString> mDidSet;
    std::deque<ATProto::AppBskyActor::ProfileView::List> mRawLists;
    std::deque<ATProto::AppBskyActor::ProfileViewDetailed::List> mRawDetailedLists;
    std::deque<ATProto::AppBskyGraph::ListItemView::List> mRawItemLists;
    std::vector<QString> mActiveFollowsDids;
    int mCountDelta = 0;
};

}
