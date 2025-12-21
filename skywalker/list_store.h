// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile_store.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class IListStore : public IProfileStore
{
public:
    using SuccessCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    virtual ~IListStore() = default;

    virtual void clear() = 0;
    virtual void loadList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb,
                  int maxPages = 2, int pagesLoaded = 0, const QString& cursor = {}) = 0;
    virtual void addList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb) = 0;

    // Creates a list entry with an empty store before loading the list members.
    virtual void addList(const ListViewBasic& list, const SuccessCb& successCb, const ErrorCb& errorCb) = 0;

    virtual void removeList(const QString& uri) = 0;
    virtual void addProfile(const QString& uri, const BasicProfile& profile, const QString& listItemUri) = 0;
    virtual void removeProfile(const QString& uri, const QString& listItemUri) = 0;

    virtual bool hasList(const QString& uri) const = 0;
    virtual QStringList getListUris() const = 0;

    virtual bool containsListMember(const QString& listUri, const QString& did) const = 0;
    virtual QStringList getListUrisForDid(const QString& did) const = 0;
    virtual const ListViewBasic& getList(const QString& uri) const = 0;
    virtual QString getListName(const QString& uri) const = 0;
};

class ListStore : public WrappedSkywalker,
                  public IListStore,
                  public Presence
{
    Q_OBJECT

public:
    static const ListStore NULL_STORE;

    explicit ListStore(QObject* parent = nullptr);

    void clear() override;
    void loadList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb,
                  int maxPages = 2, int pagesLoaded = 0, const QString& cursor = {}) override;
    void addList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb) override;

    // Creates a list entry with an empty store before loading the list members.
    void addList(const ListViewBasic& list, const SuccessCb& successCb, const ErrorCb& errorCb) override;

    void removeList(const QString& uri) override;
    void addProfile(const QString& uri, const BasicProfile& profile, const QString& listItemUri) override;
    void removeProfile(const QString& uri, const QString& listItemUri) override;

    Q_INVOKABLE bool hasList(const QString& uri) const override;
    QStringList getListUris() const override;

    bool containsListMember(const QString& listUri, const QString& did) const override;
    QStringList getListUrisForDid(const QString& did) const override;
    const ListViewBasic& getList(const QString& uri) const override;
    QString getListName(const QString& uri) const override;

    // IProfile
    Q_INVOKABLE bool contains(const QString& did) const override;
    const BasicProfile* get(const QString& did) const override;

signals:
    void listRemoved(const QString& uri);

private:
    struct ListEntry
    {
        ListViewBasic mList;
        ProfileListItemStore mStore; // List members
    };

    std::unordered_map<QString, ListEntry> mLists; // list uri -> list entry
};

}
