// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "profile_utils.h"
#include "author_cache.h"
#include "image_reader.h"
#include "photo_picker.h"
#include "skywalker.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>

namespace Skywalker {

using namespace std::chrono_literals;

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
                    qDebug() << "Reset post master";
                    mPostMaster = nullptr;
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

ATProto::PostMaster* ProfileUtils::postMaster()
{
    if (!mPostMaster)
    {
        auto* client = bskyClient();
        Q_ASSERT(client);

        if (client)
            mPostMaster = std::make_unique<ATProto::PostMaster>(*client);
        else
            qWarning() << "Bsky client not yet created";
    }

    return mPostMaster.get();
}

ImageReader* ProfileUtils::imageReader()
{
    if (!mImageReader)
        mImageReader = std::make_unique<ImageReader>(this);

    return mImageReader.get();
}

void ProfileUtils::getBasicProfile(const QString& did)
{
    const auto* profile = AuthorCache::instance().get(did);

    if (profile)
    {
        emit basicProfileOk(*profile);
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->getProfile(did,
        [this, presence=getPresence()](auto profile){
            if (!presence)
                return;

            const BasicProfile basicProfile(profile);
            AuthorCache::instance().put(basicProfile);
            emit basicProfileOk(basicProfile);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "getBasicProfile failed:" << error << " - " << msg;
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

            emit profileViewOk(Profile(profile), token);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getProfileView failed:" << error << " - " << msg;
            emit profileViewFailed(msg);
        });
}

void ProfileUtils::updateProfile(const QString& did, const QString& name, const QString& description,
                                 const QString& avatarImgSource, bool updateAvatar,
                                 const QString& bannerImgSource, bool updateBanner,
                                 const QString& pronouns, const QString& website)
{
    if (!bskyClient())
        return;

    if (updateAvatar && !avatarImgSource.isEmpty())
    {
        emit updateProfileProgress(tr("Uploading avatar"));

        QByteArray blob;
        const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit updateProfileFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), did, name, description, bannerImgSource, updateBanner, pronouns, website](auto blob){
                if (!presence)
                    return;

                continueUpdateProfile(did, name, description, std::move(blob), true, bannerImgSource, updateBanner, pronouns, website);
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
                              ATProto::Blob::SharedPtr(nullptr), updateAvatar,
                              bannerImgSource, updateBanner, pronouns, website);
    }
}

void ProfileUtils::continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                                         ATProto::Blob::SharedPtr avatarBlob, bool updateAvatar,
                                         const QString& bannerImgSource, bool updateBanner,
                                         const QString& pronouns, const QString& website)
{
    if (updateBanner && !bannerImgSource.isEmpty())
    {
        emit updateProfileProgress(tr("Uploading banner"));

        QByteArray blob;
        const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, bannerImgSource);

        if (blob.isEmpty())
        {
            emit updateProfileFailed(tr("Could not load banner image"));
            return;
        }

        mDidAvatarBlobMap[did] = std::move(avatarBlob);

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), did, name, description, updateAvatar, pronouns, website](auto blob){
                if (!presence)
                    return;

                auto avaBlob = std::move(mDidAvatarBlobMap[did]);
                mDidAvatarBlobMap.erase(did);
                continueUpdateProfile(did, name, description, std::move(avaBlob), updateAvatar, std::move(blob), true, pronouns, website);
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
                              ATProto::Blob::SharedPtr(nullptr), updateBanner,
                              pronouns, website);
    }
}

void ProfileUtils::continueUpdateProfile(const QString& did, const QString& name, const QString& description,
                                         ATProto::Blob::SharedPtr avatarBlob, bool updateAvatar,
                                         ATProto::Blob::SharedPtr bannerBlob, bool updateBanner,
                                         const QString& pronouns, const QString& website)
{
    emit updateProfileProgress(tr("Updating profile"));

    profileMaster()->updateProfile(did, name, description, std::move(avatarBlob), updateAvatar,
        std::move(bannerBlob), updateBanner, pronouns, website,
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

            const LabelerViewDetailed view(std::get<ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr>(outputView->mView));
            emit getLabelerViewDetailedOk(view);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getLabelerViewDetailed failed:" << error << " - " << msg;
            emit getLabelerViewDetailedFailed(msg);
        });
}

void ProfileUtils::likeLabeler(const QString& uri, const QString& cid)
{
    if (!postMaster())
        return;

    postMaster()->like(uri, cid, {}, {},
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            if (!presence)
                return;

            emit likeLabelerOk(likeUri);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg;
            emit likeLabelerFailed(msg);
        });
}

void ProfileUtils::undoLikeLabeler(const QString& likeUri)
{
    if (!postMaster())
        return;

    postMaster()->undo(likeUri,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            emit undoLikeLabelerOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg;
            emit undoLikeLabelerFailed(msg);
        });
}

void ProfileUtils::getFirstAppearance(const QString& did)
{
    plcDirectory().getFirstAppearance(did,
        [this, presence=getPresence(), did](QDateTime appearance){
            if (!presence)
                return;

            emit firstAppearanceOk(did, appearance);
        },
        [](int errorCode, const QString& errorMsg){
               qDebug() << "getFirstAppearance failed:" << errorCode << " - " << errorMsg;
        });
}

void ProfileUtils::setPinnedPost(const QString& did, const QString& uri, const QString& cid)
{
    if (!bskyClient())
        return;

    bskyClient()->getProfile(did,
        [this, presence=getPresence(), uri, cid](auto profile){
            if (!presence)
                return;

            if (profile->mPinnedPost)
            {
                mSkywalker->makeLocalModelChange(
                    [profile](LocalPostModelChanges* model){
                        model->updateViewerStatePinned(profile->mPinnedPost->mCid, false);
                    });
            }

            continueSetPinnedPost(profile->mDid, uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Get profile failed:" << error << " - " << msg;
            emit setPinnedPostFailed(msg);
        });
}

void ProfileUtils::continueSetPinnedPost(const QString& did, const QString& uri, const QString& cid)
{
    if (!profileMaster())
        return;

    profileMaster()->setPinnedPost(did, uri, cid,
        [this, presence=getPresence(), uri, cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateViewerStatePinned(cid, true);
                });

            emit setPinnedPostOk(uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Set pinned post failed:" << error << " - " << msg;
            emit setPinnedPostFailed(msg);
        });
}

void ProfileUtils::clearPinnedPost(const QString& did, const QString& cid)
{
    if (!profileMaster())
        return;

    profileMaster()->clearPinnedPost(did,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateViewerStatePinned(cid, false);
                });

            emit clearPinnedPostOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Clear pinned post failed:" << error << " - " << msg;
            emit clearPinnedPostFailed(msg);
        });
}

void ProfileUtils::updateStatus(const QString& did, const QString& uri, const QString& title,
                  const QString& description, const QString& thumb, int durationMinutes)
{
    qDebug() << "Update status:" << did << "uri:" << uri << "title:" << title << "thumb:" << thumb << "duration:" << durationMinutes;

    if (thumb.isEmpty())
    {
        continueUpdateStatus(did, uri, title, description, nullptr, "", durationMinutes);
        return;
    }

    imageReader()->getImage(thumb,
        [this, presence=getPresence(), did, uri, title, description, thumb, durationMinutes](auto image){
            if (presence)
                continueUpdateStatus(did, uri, title, description, image, thumb, durationMinutes);
        },
        [this, presence=getPresence(), did, uri, title, description, durationMinutes](const QString& error){
            if (!presence)
                return;

            qDebug() << "Failed to load image:" << error;
            // Update status without image anyway. Sometimes card information at sites
            // have broken image links.
            continueUpdateStatus(did, uri, title, description, nullptr, "", durationMinutes);
        });
}

void ProfileUtils::continueUpdateStatus(const QString& did, const QString& uri, const QString& title,
                          const QString& description, QImage thumb, const QString& thumbUri, int durationMinutes)
{
    Q_ASSERT(!thumb.isNull());
    QByteArray blob;
    const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, thumb, thumbUri);

    if (blob.isEmpty())
    {
        qWarning() << "Failed to create blob";
        continueUpdateStatus(did, uri, title, description, nullptr, "", durationMinutes);
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), did, uri, title, description, thumbUri, durationMinutes](auto blob){
            if (!presence)
                return;

            if (!postMaster())
                return;

            continueUpdateStatus(did, uri, title, description, blob, thumbUri, durationMinutes);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Upload blob failed:" << error << " - " << msg;
            emit updateStatusFailed(msg);
        });
}

void ProfileUtils::continueUpdateStatus(const QString& did, const QString& uri, const QString& title,
                          const QString& description, ATProto::Blob::SharedPtr thumb, const QString& thumbUri, int durationMinutes)
{
    if (!profileMaster())
        return;

    auto external = std::make_shared<ATProto::AppBskyEmbed::External>();
    external->mExternal = std::make_shared<ATProto::AppBskyEmbed::ExternalExternal>();
    external->mExternal->mUri = uri;
    external->mExternal->mTitle = title;
    external->mExternal->mDescription = description;
    external->mExternal->mThumb = thumb;

    ATProto::AppBskyActor::Status status;
    status.mStatus = ATProto::AppBskyActor::ActorStatus::LIVE;
    status.mEmbed = external;
    status.mDurationMinutes = durationMinutes;
    status.mCreatedAt = QDateTime::currentDateTimeUtc();

    profileMaster()->updateStatus(did, status,
        [this, presence=getPresence(), uri, title, description, thumbUri, durationMinutes](){
            if (!presence)
                return;

            auto externalViewExternal = std::make_shared<ATProto::AppBskyEmbed::ExternalViewExternal>();
            externalViewExternal->mUri = uri;
            externalViewExternal->mTitle = title;
            externalViewExternal->mDescription = description;

            if (!thumbUri.isEmpty())
                externalViewExternal->mThumb = thumbUri;

            auto externalView = std::make_shared<ATProto::AppBskyEmbed::ExternalView>();
            externalView->mExternal = externalViewExternal;
            auto statusView = std::make_shared<ATProto::AppBskyActor::StatusView>();
            statusView->mStatus = ATProto::AppBskyActor::ActorStatus::LIVE;
            statusView->mEmbed = externalView;
            statusView->mExpiresAt = QDateTime::currentDateTimeUtc() + durationMinutes * 1min;

            emit updateStatusOk(ActorStatusView{*statusView});
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update status failed:" << error << " - " << msg;
            emit updateStatusFailed(msg);
        });
}

void ProfileUtils::deleteStatus(const QString& did)
{
    if (!profileMaster())
        return;

    profileMaster()->deleteStatus(did,
        [this, presence=getPresence()](){
            if (!presence)
                return;

            emit deleteStatusOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Delete status failed:" << error << " - " << msg;
            emit deleteStatusFailed(msg);
        });
}

void ProfileUtils::getStatus(const QString& did)
{
    if (!profileMaster())
        return;

    profileMaster()->getStatus(did,
        [this, presence=getPresence()](ATProto::AppBskyActor::Status::SharedPtr status){
            if (!presence)
                return;

            QString uri;

            if (status->mEmbed && std::holds_alternative<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed))
            {
                auto external = std::get<ATProto::AppBskyEmbed::External::SharedPtr>(*status->mEmbed);
                uri = external->mExternal->mUri;
            }

            emit getStatusOk(uri, status->mDurationMinutes.value_or(60));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Get status failed:" << error << " - " << msg;
            emit getStatusOk("", 60);
        });
}

ActorStatusView ProfileUtils::getNullStatus()
{
    return ActorStatusView{};
}

}
