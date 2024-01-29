// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "graph_utils.h"
#include "list_view.h"
#include "local_list_model_changes.h"
#include "local_profile_changes.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class FavoriteFeeds;
class Skywalker;

class ListListModel : public QAbstractListModel,
                      public LocalListModelChanges,
                      public LocalProfileChanges
{
    Q_OBJECT
public:
    enum class Role {
        List = Qt::UserRole + 1,
        ListCreator,
        ListBlockedUri,
        ListMuted,
        ListSaved,
        ListPinned,
        MemberCheck,
        MemberListItemUri
    };

    using Type = QEnums::ListType;
    using Purpose = QEnums::ListPurpose;
    using Ptr = std::unique_ptr<ListListModel>;

    ListListModel(Type type, Purpose purpose, const QString& atId,
                  const FavoriteFeeds& favoriteFeeds, Skywalker* skywalker,
                  QObject* parent = nullptr);

    Q_INVOKABLE void setMemberCheckDid(const QString& did) { mMemberCheckDid = did; }
    Q_INVOKABLE void setExcludeInternalLists(bool exclude) { mExcludeInternalLists = exclude; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();

    // Returns the number of lists added
    int addLists(ATProto::AppBskyGraph::ListViewList lists, const QString& cursor);
    void addLists(const QList<ListView>& lists);
    Q_INVOKABLE void prependList(const ListView& list);
    Q_INVOKABLE ListView updateEntry(int index, const QString& cid, const QString& name, const QString& description, const QString& avatar);
    Q_INVOKABLE void deleteEntry(int index);
    Q_INVOKABLE ListView getEntry(int index) const;

    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Type getType() const { return mType; }
    Q_INVOKABLE Purpose getPurpose() const { return mPurpose; }
    const QString& getAtId() const { return mAtId; }

protected:
    QHash<int, QByteArray> roleNames() const override;

    // LocalListModelChanges
    virtual void blockedChanged() override;
    virtual void mutedChanged() override;
    virtual void memberListItemUriChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;

private:
    using ListList = std::deque<ListView>;

    QEnums::TripleBool memberCheck(const QString& listUri) const;
    void updateMemberCheckResults(const QString& listUri, const QString& listItemUri);
    QString getMemberListItemUri(const QString& listUri) const;
    void listSavedChanged();
    void listPinnedChanged();
    void changeData(const QList<int>& roles);

    ListList filterLists(ATProto::AppBskyGraph::ListViewList lists) const;

    Type mType;
    Purpose mPurpose;
    QString mAtId;
    ListList mLists;
    QString mCursor;
    const FavoriteFeeds& mFavoriteFeeds;
    GraphUtils mGraphUtils;
    QString mMemberCheckDid;
    std::unordered_map<QString, std::optional<QString>> mMemberCheckResults; // listUri -> listItemUri
    bool mExcludeInternalLists = false;
};

}
