// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "file_utils.h"
#include "photo_picker.h"
#include <QJsonDocument>

namespace Skywalker {

namespace {

constexpr char const* DRAFT_POSTS_DIR = "sw-draft-posts";

}

DraftPosts::DraftPosts(QObject* parent) :
    QObject(parent)
{
}

void DraftPosts::saveDraftPost(const QString& text,
                               const QStringList& imageFileNames, const QStringList& altTexts,
                               const QString& replyToUri, const QString& replyToCid,
                               const QString& replyRootUri, const QString& replyRootCid,
                               const QString& quoteUri, const QString& quoteCid,
                               const QStringList& labels)
{
    Q_ASSERT(imageFileNames.size() == altTexts.size());
    qDebug() << "Save draft post:" << text;
    const QString draftsPath = FileUtils::getAppDataPath(DRAFT_POSTS_DIR);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << DRAFT_POSTS_DIR;
        emit saveDraftPostFailed(tr("Cannot create app data path"));
        return;
    }

    const QString dateTime = FileUtils::createDateTimeName();

    ATProto::AppBskyFeed::PostReplyRef::Ptr replyRef = replyToUri.isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

    auto post = ATProto::PostMaster::createPostWithoutFacets(text, std::move(replyRef));
    ATProto::PostMaster::addLabelsToPost(*post, labels);

    if (!quoteUri.isEmpty())
        ATProto::PostMaster::addQuoteToPost(*post, quoteUri, quoteCid);

    for (int i = 0; i < imageFileNames.size(); ++i)
    {
        const QString& imgName = imageFileNames[i];
        auto blob = saveImage(imgName, draftsPath, dateTime, i);

        if (!blob)
        {
            // Cleanup already saved images
            for (int j = 0; j < i; ++j)
                dropImage(draftsPath, dateTime, j);

            return;
        }

        const QString& altText = altTexts[i];
        ATProto::PostMaster::addImageToPost(*post, std::move(blob), altText);
    }

    // TODO threadgate

    if (!save(*post, draftsPath, dateTime))
    {
        // Cleanup already saved images
        for (int j = 0; j < imageFileNames.size(); ++j)
            dropImage(draftsPath, dateTime, j);
    }
}

QString DraftPosts::createDraftPostFileName(const QString& baseName) const
{
    return QString("SWP_%1.json").arg(baseName);
}

QString DraftPosts::createDraftImageFileName(const QString& baseName, int seq) const
{
    return QString("SWI%1_%2.jpg").arg(seq).arg(baseName);
}

bool DraftPosts::save(const ATProto::AppBskyFeed::Record::Post& post, const QString& draftsPath, const QString& baseName)
{
    const QString postFileName = createDraftPostFileName(baseName);
    const QString fileName = QString("%1/%2").arg(draftsPath, postFileName);
    qDebug() << "Draft post name:" << fileName;
    QFile file(fileName);

    if (file.exists())
    {
        qWarning() << "File already exists:" << fileName;
        emit saveDraftPostFailed(tr("File already exists: %1").arg(fileName));
        return false;
    }

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot create file:" << fileName;
        emit saveDraftPostFailed(tr("Cannot create file: %1").arg(fileName));
        return false;
    }

    const auto json = QJsonDocument(post.toJson());
    const QByteArray data = json.toJson(QJsonDocument::Compact);

    if (file.write(data) == -1)
    {
        qWarning() << "Failed to write:" << fileName;
        emit saveDraftPostFailed(tr("Failed to write: %1").arg(fileName));
        return false;
    }

    file.close();
    emit saveDraftPostOk();
    return true;
}

ATProto::Blob::Ptr DraftPosts::saveImage(const QString& imgName, const QString& draftsPath,
                                         const QString& baseName, int seq)
{
    qDebug() << "Save image:" << seq << imgName << "path:" << draftsPath << "base:" << baseName;
    QImage img = PhotoPicker::loadImage(imgName);

    if (img.isNull())
    {
        qWarning() << "Could not load image:" << seq << imgName;
        emit saveDraftPostFailed(tr("Could not load image #%1: %2").arg(seq + 1).arg(imgName));
        return nullptr;
    }

    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = QString("%1/%2").arg(draftsPath, imgFileName);
    qDebug() << "Draft image file name:" << fileName;

    if (!img.save(fileName))
    {
        qWarning() << "Failed to save image:" << fileName;
        emit saveDraftPostFailed(tr("Failed to save image #%1: %2").arg(seq + 1).arg(fileName));
        return nullptr;
    }

    auto blob = std::make_unique<ATProto::Blob>();
    blob->mRefLink = imgFileName;
    blob->mMimeType = "image/jpeg";
    blob->mSize = img.sizeInBytes();
    return blob;
}

void DraftPosts::dropImage(const QString& draftsPath, const QString& baseName, int seq)
{
    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = QString("%1/%2").arg(draftsPath, imgFileName);
    qDebug() << "Drop draft image:" << fileName;
    QFile::remove(fileName);
}

}
