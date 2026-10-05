// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "local_list_model_changes.h"
#include "starter_pack.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class StarterPackListModel : public QAbstractListModel,
                             public BaseListModel,
                             public LocalListModelChanges
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    enum class Role {
        StarterPack = Qt::UserRole,
        MemberCountDelta,
        MemberCheck,
        MemberListItemUri
    };

    using Ptr = std::unique_ptr<StarterPackListModel>;

    explicit StarterPackListModel(QObject* parent = nullptr);

    Q_INVOKABLE void setMemberCheckDid(const QString& did) { mMemberCheckDid = did; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();

    // The lists are sorted by name before the are added.
    // Sorting per page is not ideal. By retrieving large pages (100 entries), it should work
    // fine for most users.
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasic::List starterPacks, const QString& cursor);
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackView::List starterPacks, const QString& cursor);
    void addStarterPacks(ATProto::AppBskyGraph::StarterPackWithMembership::List starterPacksWithMembership, const QString& cursor);

    Q_INVOKABLE void prependStarterPack(const StarterPackView& starterPack);
    Q_INVOKABLE StarterPackViewBasic updateEntry(int index, const QString& cid, const QString& name,
                                                 const QString& description, const NamedLink::List& embeddedLinks);
    Q_INVOKABLE void deleteEntry(int index);
    Q_INVOKABLE StarterPackViewBasic getEntry(int index) const;

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    bool needsMembershipInfo() const { return !mMemberCheckDid.isEmpty(); }
    const QString& getMemberCheckDid() const { return mMemberCheckDid; }

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    // LocalListModelChanges
    virtual void blockedChanged() override {}
    virtual void mutedChanged() override {}
    virtual void hideFromTimelineChanged() override {}
    virtual void syncListChanged() override {}
    virtual void hideRepliesChanged() override {}
    virtual void hideFollowingChanged() override {}
    virtual void memberListItemUriChanged() override;


private:
    template<typename T>
    void _addStarterPacks(const std::vector<T>& starterPacks, const QString& cursor);

    using StarterPackList = std::deque<StarterPackView>;

    QEnums::TripleBool memberCheck(const QString& starterPackUri) const;
    QString getMemberListItemUri(const QString& starterPackUri) const;
    void changeData(const QList<int>& roles) override;

    StarterPackList mStarterPacks;
    QString mCursor;
    QString mMemberCheckDid;
    std::unordered_map<QString, QString> mMemberCheckResults; // starterPackUri -> listItemUri
    bool mGetFeedInProgress = false;
};

}
