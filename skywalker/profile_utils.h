// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/profile_master.h>

namespace Skywalker {

class ProfileUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProfileUtils(QObject* parent = nullptr);

    // The token will be returned in the emitted signal
    Q_INVOKABLE void getProfileView(const QString& atId, const QString& token);

    // ImgSource must be a 'file://' or 'image://' reference.
    Q_INVOKABLE void updateProfile(const QString& did, const QString& name, const QString& description,
                                   const QString& avatarImgSource, bool updateAvatar,
                                   const QString& bannerImgSource, bool updateBanner);

signals:
    void profileViewOk(Profile profile, const QString& token);
    void profileViewFailed(QString error);
    void updateProfileProgress(QString msg);
    void updateProfileOk();
    void updateProfileFailed(QString error);

private:
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                               const QString& bannerImgSource, bool updateBanner);
    void continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                               ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                               ATProto::Blob::Ptr bannerBlob, bool updateBanner);

    ATProto::ProfileMaster* profileMaster();
    std::unique_ptr<ATProto::ProfileMaster> mProfileMaster;
    std::unordered_map<QString, ATProto::Blob::Ptr> mDidAvatarBlobMap;
};

}
