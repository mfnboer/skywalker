// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile_store.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ListStore : public WrappedSkywalker, public IProfileStore, public Presence
{
    Q_OBJECT

public:
    using SuccessCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    explicit ListStore(QObject* parent = nullptr);

    void clear();
    void loadList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb,
                  int maxPages = 2, int pagesLoaded = 0, const QString& cursor = {});
    void addList(const QString& uri, const SuccessCb& successCb, const ErrorCb& errorCb);
    void removeList(const QString& uri);
    void addProfile(const QString& uri, const BasicProfile& profile, const QString& listItemUri);
    void removeProfile(const QString& uri, const QString& listItemUri);
    Q_INVOKABLE bool hasList(const QString& uri) const;
    QStringList getListUris() const;

    Q_INVOKABLE bool contains(const QString& did) const override;
    const BasicProfile* get(const QString& did) const override;
    ScopedHandle* registerRemovedCb(const RemovedCb&, QObject*) override { Q_ASSERT(false); return nullptr; }

private:
    std::unordered_map<QString, ProfileListItemStore> mLists; // list uri -> list members
};

}
