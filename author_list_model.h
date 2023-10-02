// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "local_author_model_changes.h"
#include "profile.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class AuthorListModel : public QAbstractListModel, public LocalAuthorModelChanges
{
    Q_OBJECT
public:
    enum class Role {
        Author = Qt::UserRole + 1,
        FollowingUri
    };

    using Type = QEnums::AuthorListType;

    using Ptr = std::unique_ptr<AuthorListModel>;

    AuthorListModel(Type type, const QString& atId, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addAuthors(ATProto::AppBskyActor::ProfileViewList authors, const QString& cursor);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Type getType() const { return mType; }
    const QString& getAtId() const { return mAtId; }

protected:
    QHash<int, QByteArray> roleNames() const override;

    virtual void followingUriChanged() override;

private:
    void changeData(const QList<int>& roles);

    Type mType;

    using AuthorList = std::deque<Profile>;
    AuthorList mList;
    std::deque<ATProto::AppBskyActor::ProfileViewList> mRawLists;

    QString mCursor;
    QString mAtId;
};

}
