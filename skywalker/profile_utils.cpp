// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "profile_utils.h"
#include "author_cache.h"
#include "photo_picker.h"
#include "skywalker.h"

namespace Skywalker {

ProfileUtils::ProfileUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    connect(this, &WrappedSkywalker::skywalkerChanged, this, [this]{
        if (!mSkywalker)
            return;

        connect(mSkywalker, &Skywalker::bskyClientDeleted, this,
                [this]{
                    qDebug() << "Reset profile master";
                    mProfileMaster = nullptr;
                });
    });
}

ATProto::ProfileMaster* ProfileUtils::profileMaster()
{
    if (!mProfileMaster)
    {
        auto* client = bskyClient();
        Q_ASSERT(client);

        if (client)
            mProfileMaster = std::make_unique<ATProto::ProfileMaster>(*bskyClient());
        else
            qWarning() << "Bsky client not yet created";
    }

    return mProfileMaster.get();
}

void ProfileUtils::getHandle(const QString& did)
{
    const auto* profile = AuthorCache::instance().get(did);

    if (profile)
    {
        emit handle(profile->getHandle(), profile->getDisplayName(), did);
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->getProfile(did,
        [this, presence=getPresence()](auto profile){
            if (!presence)
                return;

            AuthorCache::instance().put(BasicProfile(profile.get()));
            emit handle(profile->mHandle, profile->mDisplayName.value_or(""), profile->mDid);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "getProfileView failed:" << error << " - " << msg;
        });
}

void ProfileUtils::getProfileView(const QString& atId, const QString& token)
{
    if (!bskyClient())
        return;

    bskyClient()->getProfile(atId,
        [this, presence=getPresence(), token](auto profile){
            if (!presence)
                return;

            auto shared = ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr(profile.release());
            emit profileViewOk(Profile(shared), token);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getProfileView failed:" << error << " - " << msg;
            emit profileViewFailed(msg);
        });
}

void ProfileUtils::updateProfile(const QString& did, const QString& name, const QString& description,
                                 const QString& avatarImgSource, bool updateAvatar,
                                 const QString& bannerImgSource, bool updateBanner)
{
    if (!bskyClient())
        return;

    if (updateAvatar && !avatarImgSource.isEmpty())
    {
        emit updateProfileProgress(tr("Uploading avatar"));

        QByteArray blob;
        const QString mimeType = PhotoPicker::createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit updateProfileFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), did, name, description, bannerImgSource, updateBanner](auto blob){
                if (!presence)
                    return;

                continueUpdateProfile(did, name, description, std::move(blob), true, bannerImgSource, updateBanner);
            },
            [this, presence=getPresence()](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qDebug() << "Update profile failed:" << error << " - " << msg;
                emit updateProfileFailed(msg);
            });
    }
    else
    {
        continueUpdateProfile(did, name, description,
                              ATProto::Blob::Ptr(nullptr), updateAvatar,
                              bannerImgSource, updateBanner);
    }
}

void ProfileUtils::continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                                         ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                                         const QString& bannerImgSource, bool updateBanner)
{
    if (updateBanner && !bannerImgSource.isEmpty())
    {
        emit updateProfileProgress(tr("Uploading banner"));

        QByteArray blob;
        const QString mimeType = PhotoPicker::createBlob(blob, bannerImgSource);

        if (blob.isEmpty())
        {
            emit updateProfileFailed(tr("Could not load banner image"));
            return;
        }

        mDidAvatarBlobMap[did] = std::move(avatarBlob);

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), did, name, description, updateAvatar](auto blob){
                if (!presence)
                    return;

                auto avaBlob = std::move(mDidAvatarBlobMap[did]);
                mDidAvatarBlobMap.erase(did);
                continueUpdateProfile(did, name, description, std::move(avaBlob), updateAvatar, std::move(blob), true);
            },
            [this, presence=getPresence()](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qDebug() << "Update profile failed:" << error << " - " << msg;
                emit updateProfileFailed(msg);
            });
    }
    else
    {
        continueUpdateProfile(did, name, description,
                              std::move(avatarBlob), updateAvatar,
                              ATProto::Blob::Ptr(nullptr), updateBanner);
    }
}

void ProfileUtils::continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                                         ATProto::Blob::Ptr avatarBlob, bool updateAvatar,
                                         ATProto::Blob::Ptr bannerBlob, bool updateBanner)
{
    emit updateProfileProgress(tr("Updating profile"));

    profileMaster()->updateProfile(did, name, description, std::move(avatarBlob), updateAvatar,
        std::move(bannerBlob), updateBanner,
        [this, presence=getPresence()](){
            if (!presence)
                return;

            emit updateProfileOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update profile failed:" << error << " - " << msg;
            emit updateProfileFailed(msg);
        });
}

void ProfileUtils::getLabelerViewDetailed(const QString& did)
{
    if (!bskyClient())
        return;

    qDebug() << "Get detailed labeler view:" << did;

    bskyClient()->getServices({did}, true,
        [this](auto output){
            if (output->mViews.empty())
            {
                qWarning() << "Invalid services output, views missing";
                emit getLabelerViewDetailedFailed(tr("Could not get label information."));
                return;
            }

            const auto& outputView = output->mViews.front();

            if (outputView->mViewType != ATProto::AppBskyLabeler::GetServicesOutputView::ViewType::VIEW_DETAILED)
            {
                qWarning() << "Invalid view type:" << (int)outputView->mViewType;
                emit getLabelerViewDetailedFailed(tr("Could not get label information."));
                return;
            }

            const LabelerViewDetailed view(*std::get<ATProto::AppBskyLabeler::LabelerViewDetailed::Ptr>(outputView->mView));
            emit getLabelerViewDetailedOk(view);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getLabelerViewDetailed failed:" << error << " - " << msg;
            emit getLabelerViewDetailedFailed(msg);
        });
}

}
