// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "list_view.h"
#include "enums.h"
#include "presence.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/graph_master.h>

namespace Skywalker {

class GraphUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GraphUtils(QObject* parent = nullptr);

    Q_INVOKABLE void follow(const BasicProfile& profile);
    Q_INVOKABLE void unfollow(const QString& did, const QString& followingUri);
    Q_INVOKABLE void block(const QString& did);
    Q_INVOKABLE void unblock(const QString& did, const QString& blockingUri);
    Q_INVOKABLE void mute(const QString& did);
    Q_INVOKABLE void unmute(const QString& did);

    // avatarImgSource must be a 'file://' or 'image://' reference.
    Q_INVOKABLE void createList(const QEnums::ListPurpose purpose, const QString& name,
                                const QString& description, const QString& avatarImgSource);
    Q_INVOKABLE void updateList(const QString& listUri, const QString& name,
                                const QString& description, const QString& avatarImgSource,
                                bool updateAvatar);
    Q_INVOKABLE void deleteList(const QString& listUri);
    Q_INVOKABLE void getListView(const QString& listUri, bool viewPosts = false);
    Q_INVOKABLE void addListUser(const QString& listUri, const BasicProfile& profile);
    Q_INVOKABLE void removeListUser(const QString& listUri, const QString& listItemUri);
    Q_INVOKABLE void isListUser(const QString& listUri, const QString& did, int maxPages = 10, const std::optional<QString> cursor = {});

    Q_INVOKABLE ListView makeListView(const QString& uri, const QString& cid, const QString& name,
                    QEnums::ListPurpose purpose, const QString& avatar,
                    const Profile& creator, const QString& description);

    Q_INVOKABLE void blockList(const QString& listUri);
    Q_INVOKABLE void unblockList(const QString& listUri, const QString& blockingUri);
    Q_INVOKABLE void muteList(const QString& listUri);
    Q_INVOKABLE void unmuteList(const QString& listUri);

    Q_INVOKABLE bool areRepostsMuted(const QString& did) const;
    Q_INVOKABLE void muteReposts(const BasicProfile& profile);
    Q_INVOKABLE void unmuteReposts(const QString& did);

    // Check if a list is a list internally used by Skywalker
    bool isInternalList(const QString& listUri) const;

signals:
    void followOk(QString uri);
    void followFailed(QString error);
    void unfollowOk();
    void unfollowFailed(QString error);
    void blockOk(QString uri);
    void blockFailed(QString error);
    void unblockOk();
    void unblockFailed(QString error);
    void muteOk();
    void muteFailed(QString error);
    void unmuteOk();
    void unmuteFailed(QString error);
    void createListProgress(QString msg);
    void createListOk(QString uri, QString cid);
    void createListFailed(QString error);
    void updateListProgress(QString msg);
    void updateListOk(QString uri, QString cid);
    void updateListFailed(QString error);
    void deleteListOk();
    void deleteListFailed(QString error);
    void getListOk(ListView list, bool viewPosts);
    void getListFailed(QString error);
    void addListUserOk(QString did, QString itemUri, QString itemCid);
    void addListUserFailed(QString error);
    void removeListUserOk();
    void removeListUserFailed(QString error);
    void isListUserOk(QString listUri, QString did, QString listItemUri);
    void isListUserFailed(QString error);
    void blockListOk(QString uri);
    void blockListFailed(QString error);
    void unblockListOk();
    void unblockListFailed(QString error);
    void muteListOk();
    void muteListFailed(QString error);
    void unmuteListOk();
    void unmuteListFailed(QString error);
    void muteRepostsOk();
    void muteRepostsFailed(QString error);
    void unmuteRepostsOk();
    void unmuteRepostsFailed(QString error);

private:
    void continueCreateList(const QEnums::ListPurpose purpose, const QString& name,
                            const QString& description, ATProto::Blob::SharedPtr blob);
    void continueUpdateList(const QString& listUri, const QString& name,
                            const QString& description, ATProto::Blob::SharedPtr blob, bool updateAvatar);

    ATProto::GraphMaster* graphMaster();
    std::unique_ptr<ATProto::GraphMaster> mGraphMaster;
};

}
