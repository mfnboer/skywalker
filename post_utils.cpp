// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"

namespace Skywalker {

PostUtils::PostUtils(QObject* parent) :
    QObject(parent)
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();
    QObject::connect(&jniCallbackListener, &JNICallbackListener::photoPicked,
                     this, [this](const QString& uri){
                         qDebug() << "PHOTO PICKED:" << uri;
                         QString fileName = resolveContentUriToFile(uri);
                         qDebug() << "PHOTO FILE NAME:" << fileName;
                         QFile file(fileName);
                         qDebug() << "File exists:" << file.exists() << ",size:" << file.size();
                         emit photoPicked(fileName);
                     });
}

ATProto::Client* PostUtils::bskyClient()
{
    Q_ASSERT(mSkywalker);
    auto* client = mSkywalker->getBskyClient();
    Q_ASSERT(client);
    return client;
}

void PostUtils::setSkywalker(Skywalker* skywalker)
{
    mSkywalker = skywalker;
    emit skywalkerChanged();
}

void PostUtils::post(QString text, const QStringList& imageFileNames)
{
    Q_ASSERT(mSkywalker);
    text.replace("\u00A0", " "); // replace nbsp by normal space
    qDebug() << "Posting:" << text;

    bskyClient()->createPost(text, [this, imageFileNames](auto post){
        continuePost(imageFileNames, post); });
}

void PostUtils::continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex)
{
    if (imgIndex >= imageFileNames.size())
    {
        emit postProgress(tr("Posting"));
        bskyClient()->post(*post,
            [this]{
                emit postOk();
            },
            [this](const QString& error){
                qDebug() << "Post failed:" << error;
                emit postFailed(error);
            });

        return;
    }

    const auto& fileName = imageFileNames[imgIndex];
    const auto& blob = createBlob(fileName);
    if (blob.isEmpty())
    {
        const QString error = tr("Could not load image") + ": " + QFileInfo(fileName).fileName();
        emit postFailed(error);
        return;
    }

    emit postProgress(tr("Uploading image") + QString(" #%1").arg(imgIndex + 1));

    bskyClient()->uploadBlob(blob, "image/jpeg",
        [this, imageFileNames, post, imgIndex](auto blob){
            bskyClient()->addImageToPost(*post, std::move(blob));
            continuePost(imageFileNames, post, imgIndex + 1);
        },
        [this](const QString& error){
            qDebug() << "Post failed:" << error;
            emit postFailed(error);
        });
}

void PostUtils::pickPhoto()
{
    ::Skywalker::pickPhoto();
}

void PostUtils::setFirstWebLink(const QString& link)
{
    if (link == mFirstWebLink)
        return;

    mFirstWebLink = link;
    emit firstWebLinkChanged();
}

QString PostUtils::highlightMentionsAndLinks(const QString& text)
{
    const auto facets = bskyClient()->parseFacets(text);
    QString highlighted;
    int pos = 0;
    bool webLinkFound = false;
    mLinkShorteningReduction = 0;

    for (const auto& facet : facets)
    {
        if (facet.mType == ATProto::Client::ParsedMatch::Type::LINK)
        {
            if (!webLinkFound)
            {
                setFirstWebLink(facet.mMatch);
                webLinkFound = true;
            }

            const auto shortLink = ATProto::Client::shortenWebLink(facet.mMatch);
            const int reduction = graphemeLength(facet.mMatch) - graphemeLength(shortLink);
            qDebug() << "SHORT:" << shortLink << "reduction:" << reduction;
            mLinkShorteningReduction += reduction;
        }

        const auto before = text.sliced(pos, facet.mStartIndex - pos);
        highlighted.append(before.toHtmlEscaped().replace(' ', "&nbsp;"));
        QString highlight = QString("<font color=\"blue\">%1</font>").arg(facet.mMatch);
        highlighted.append(highlight);
        pos = facet.mEndIndex;
    }

    if (!webLinkFound)
        setFirstWebLink(QString());

    highlighted.append(text.sliced(pos).toHtmlEscaped().replace(' ', "&nbsp;"));
    return highlighted;
}

int PostUtils::graphemeLength(const QString& text)
{
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    int length = 0;

    while (boundaryFinder.toNextBoundary() != -1)
        ++length;

    return length;
}

}
