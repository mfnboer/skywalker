// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_migration.h"
#include "atproto_image_provider.h"
#include "skywalker.h"

namespace Skywalker {

DraftPostsMigration::DraftPostsMigration(Skywalker* skywalker, QObject* parent) :
    QObject(parent),
    mSKywalker(skywalker),
    mRepoDrafts(this),
    mFileDrafts(this)
{
    mRepoDrafts.setSkywalker(skywalker);
    mRepoDrafts.setStorageType(DraftPosts::STORAGE_REPO);
    mFileDrafts.setSkywalker(skywalker);
    mFileDrafts.setStorageType(DraftPosts::STORAGE_FILE);

    connect(&mRepoDrafts, &DraftPosts::loadDraftPostsOk, this, [this]{ loadImagesFromRepo(); });
    connect(&mRepoDrafts, &DraftPosts::loadDraftPostsFailed, this, [this](QString){ emit migrationFailed(); });
}

void DraftPostsMigration::migrateFromRepoToFile()
{
    qDebug() << "Migrate draft posts from repo to local files";
    mRepoDrafts.loadDraftPosts();
}

void DraftPostsMigration::loadImagesFromRepo()
{
    DraftPostsModel* model = mRepoDrafts.getDraftPostsModel();

    if (!model)
    {
        qWarning() << "No model!";
        emit migrationFailed();
        return;
    }

    QStringList images;

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const Post& post = model->getPost(i);

        for (const auto& imageView : post.getImages())
            images.push_back(imageView.getFullSizeUrl());
    }

    loadImagesFromRepo(images);
}

void DraftPostsMigration::loadImagesFromRepo(const QStringList& images)
{
    if (images.isEmpty())
    {
        migrateToFile();
        return;
    }

    auto* imgProvider = ATProtoImageProvider::getProvider(ATProtoImageProvider::DRAFT_IMAGE);
    QString img = images.front();

    imgProvider->asyncAddImage(img, [this, remainingImages=images.mid(1)]{
        loadImagesFromRepo(remainingImages); });
}

void DraftPostsMigration::migrateToFile()
{
    DraftPostsModel* model = mRepoDrafts.getDraftPostsModel();

    if (!model)
    {
        qWarning() << "No model!";
        emit migrationFailed();
        return;
    }

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    QStringList savedPostUris;
    bool allDraftsMigrated = true;

    for (int i = model->rowCount() - 1; i >= 0; --i)
    {
        const DraftPostData* data = mRepoDrafts.getDraftPostData(i);
        const auto [imageFileNames, altTexts] = getImages(*data);

        const bool saved = mFileDrafts.saveDraftPost(
            data->text(),
            imageFileNames, altTexts,
            data->replyToUri(), data->replyToCid(),
            data->replyRootUri(), data->replyRootCid(),
            data->replyToAuthor(), data->replyToText(),
            data->replyToDateTime(),
            data->quoteUri(), data->quoteCid(),
            data->quoteAuthor(), data->quoteText(),
            data->quoteDateTime(),
            data->quoteFeed(), data->quoteList(),
            data->gif(), data->labels(),
            data->restrictReplies(), data->allowMention(), data->allowFollowing(),
            data->allowLists(),
            data->indexedAt());

        for (const auto& imgSource : imageFileNames)
            imgProvider->removeImage(imgSource);

        if (saved)
        {
            savedPostUris.push_back(data->recordUri());
        }
        else
        {
            qWarning() << "Could not save draft:" << data->recordUri();
            allDraftsMigrated = false;
        }
    }

    deleteRecords(savedPostUris,
        [this, allDraftsMigrated]{
            if (allDraftsMigrated)
                emit migrationOk();
            else
                emit migrationFailed();
        },
        [this]{
            emit migrationFailed();
        });
}

void DraftPostsMigration::deleteRecords(const QStringList& recordUris,
                                        const std::function<void()>& okCb,
                                        const std::function<void()>& failCb)
{
    if (recordUris.isEmpty())
    {
        okCb();
        return;
    }

    bool allUrisValid = true;
    ATProto::ComATProtoRepo::ApplyWritesList writes;

    for (const auto& uri : recordUris)
    {
        ATProto::ATUri atUri(uri);

        if (!atUri.isValid())
        {
            qWarning() << "Invalid record uri:" << uri;
            allUrisValid = false;
            continue;
        }

        qDebug() << "Delete record:" << uri;
        auto deleteRecord = std::make_unique<ATProto::ComATProtoRepo::ApplyWritesDelete>();
        deleteRecord->mCollection = atUri.getCollection();
        deleteRecord->mRKey = atUri.getRkey();
        writes.push_back(std::move(deleteRecord));
    }

    if (writes.empty())
    {
        failCb();
        return;
    }

    const QString repo = mSKywalker->getUserDid();

    mSKywalker->getBskyClient()->applyWrites(repo, writes, false,
        [okCb, failCb, allUrisValid]{
            if (allUrisValid)
                okCb();
            else
                failCb();
        },
        [failCb](const QString& error, const QString& msg) {
            qWarning() << "Failed to delete records:" << error << "-" << msg;
            failCb();
        });
}

std::tuple<QStringList, QStringList> DraftPostsMigration::getImages(const DraftPostData& data) const
{
    QStringList imageFileNames;
    QStringList altTexts;

    for (const auto& imageView : data.images())
    {
        imageFileNames.push_back(imageView.getFullSizeUrl());
        altTexts.push_back(imageView.getAlt());
    }

    return { imageFileNames, altTexts };
}

}
