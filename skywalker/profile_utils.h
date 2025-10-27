// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "labeler.h"
#include "presence.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/post_master.h>
#include <atproto/lib/profile_master.h>

namespace Skywalker {

class ProfileUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProfileUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getBasicProfile(const QString& did);

    // The token will be returned in the emitted signal
    // atId is handle or did.
    Q_INVOKABLE void getProfileView(const QString& atId, const QString& token);

    // ImgSource must be a 'file://' or 'image://' reference.
    Q_INVOKABLE void updateProfile(const QString& did, const QString& name, const QString& description,
                                   const QString& avatarImgSource, bool updateAvatar,
                                   const QString& bannerImgSource, bool updateBanner,
                                   const QString& pronouns, const QString& website);

    Q_INVOKABLE void getLabelerViewDetailed(const QString& did);
    Q_INVOKABLE void likeLabeler(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoLikeLabeler(const QString& likeUri);
    Q_INVOKABLE void getFirstAppearance(const QString& did);
    Q_INVOKABLE void setPinnedPost(const QString& did, const QString& uri, const QString& cid);
    Q_INVOKABLE void clearPinnedPost(const QString& did, const QString& cid);

signals:
    void basicProfileOk(BasicProfile profile);
    void profileViewOk(Profile profile, const QString& token);
    void profileViewFailed(QString error);
    void updateProfileProgress(QString msg);
    void updateProfileOk();
    void updateProfileFailed(QString error);
    void getLabelerViewDetailedOk(LabelerViewDetailed);
    void getLabelerViewDetailedFailed(QString error);
    void likeLabelerOk(QString likeUri);
    void likeLabelerFailed(QString error);
    void undoLikeLabelerOk();
    void undoLikeLabelerFailed(QString error);
    void firstAppearanceOk(QString did, QDateTime appearance);
    void setPinnedPostOk(QString uri, QString cid);
    void setPinnedPostFailed(QString error);
    void clearPinnedPostOk();
    void clearPinnedPostFailed(QString error);

private:
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::SharedPtr avatarBlob, bool updateAvatar,
                               const QString& bannerImgSource, bool updateBanner,
                               const QString& pronouns, const QString& website);
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::SharedPtr avatarBlob, bool updateAvatar,
                               ATProto::Blob::SharedPtr bannerBlob, bool updateBanner,
                               const QString& pronouns, const QString& website);
    void continueSetPinnedPost(const QString& did, const QString& uri, const QString& cid);

    ATProto::ProfileMaster* profileMaster();
    ATProto::PostMaster* postMaster();

    std::unique_ptr<ATProto::ProfileMaster> mProfileMaster;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    std::unordered_map<QString, ATProto::Blob::SharedPtr> mDidAvatarBlobMap;
};

}
