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

    Q_INVOKABLE void getHandle(const QString& did);

    // The token will be returned in the emitted signal
    // atId is handle or did.
    Q_INVOKABLE void getProfileView(const QString& atId, const QString& token);

    // ImgSource must be a 'file://' or 'image://' reference.
    Q_INVOKABLE void updateProfile(const QString& did, const QString& name, const QString& description,
                                   const QString& avatarImgSource, bool updateAvatar,
                                   const QString& bannerImgSource, bool updateBanner);

    Q_INVOKABLE void getLabelerViewDetailed(const QString& did);
    Q_INVOKABLE void likeLabeler(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoLikeLabeler(const QString& likeUri, const QString& cid);
    Q_INVOKABLE void getFirstAppearance(const QString& did);

signals:
    void handle(QString handle, QString displayName, QString did);
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

private:
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                               const QString& bannerImgSource, bool updateBanner);
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                               ATProto::Blob::Ptr bannerBlob, bool updateBanner);

    ATProto::ProfileMaster* profileMaster();
    ATProto::PostMaster* postMaster();

    std::unique_ptr<ATProto::ProfileMaster> mProfileMaster;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    std::unordered_map<QString, ATProto::Blob::Ptr> mDidAvatarBlobMap;
};

}
