// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_view.h"
#include <QAbstractListModel>
#include <vector>

namespace Skywalker
{

class ConvoListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool getConvosInProgress READ isGetConvosInProgress NOTIFY getConvosInProgressChanged FINAL)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY loadedChanged FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount NOTIFY unreadCountChanged FINAL)

public:
    enum class Role {
        Convo = Qt::UserRole + 1,
        EndOfList
    };

    using Ptr = std::unique_ptr<ConvoListModel>;

    explicit ConvoListModel(const QString& userDid, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addConvos(const ATProto::ChatBskyConvo::ConvoViewList& convos, const QString& cursor);
    void updateConvo(const ATProto::ChatBskyConvo::ConvoView& convo);
    void insertConvo(const ConvoView& convo);
    void deleteConvo(const QString& convoId);
    const ConvoView* getConvo(const QString& convoId) const;
    bool hasConvo(const QString& convoId) const { return getConvo(convoId) != nullptr; }
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }
    QString getLastRev() const;
    BasicProfileList getAllConvoMembers() const;

    void setGetConvosInProgress(bool inProgress);
    bool isGetConvosInProgress() const { return mGetConvosInProgress; }
    void setLoaded(bool loaded);
    bool isLoaded() const { return mLoaded; }

    void setUnreadCount(int unread);
    void updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output);
    int getUnreadCount() const { return mUnreadCount; }

signals:
    void getConvosInProgressChanged();
    void loadedChanged();
    void unreadCountChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void changeData(const QList<int>& roles, int begin = 0, int end = -1);
    bool checkIndex(int index) const;

    const QString& mUserDid;
    std::vector<ConvoView> mConvos;
    std::unordered_map<QString, int> mConvoIdIndexMap;
    QString mCursor;
    bool mGetConvosInProgress = false;
    bool mLoaded = false;
    int mUnreadCount = 0;
};

}
