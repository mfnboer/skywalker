// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"

namespace Skywalker {

PostUtils::PostUtils(QObject* parent) :
    QObject(parent),
    Presence()
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

ATProto::PostMaster* PostUtils::postMaster()
{
    if (!mPostMaster)
    {
        Q_ASSERT(mSkywalker);
        Q_ASSERT(mSkywalker->getBskyClient());
        mPostMaster = std::make_unique<ATProto::PostMaster>(*mSkywalker->getBskyClient());
    }

    return mPostMaster.get();
}

ImageReader* PostUtils::imageReader()
{
    if (!mImageReader)
        mImageReader = std::make_unique<ImageReader>();

    return mImageReader.get();
}

void PostUtils::setSkywalker(Skywalker* skywalker)
{
    Q_ASSERT(skywalker);
    mSkywalker = skywalker;
    emit skywalkerChanged();
}

static ATProto::AppBskyFeed::PostReplyRef::Ptr createReplyRef(
        const QString& replyToUri, const QString& replyToCid,
        const QString& replyRootUri, const QString& replyRootCid)
{
    if (replyToUri.isEmpty() || replyToCid.isEmpty())
        return nullptr;

    auto replyRef = std::make_unique<ATProto::AppBskyFeed::PostReplyRef>();

    replyRef->mParent = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mParent->mUri = replyToUri;
    replyRef->mParent->mCid = replyToCid;

    replyRef->mRoot = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mRoot->mUri = replyRootUri.isEmpty() ? replyToUri : replyRootUri;
    replyRef->mRoot->mCid = replyRootCid.isEmpty() ? replyToCid : replyRootCid;

    return replyRef;
}

void PostUtils::post(QString text, const QStringList& imageFileNames,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid)
{
    qDebug() << "Posting:" << text;

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, nullptr,
            [this, presence=getPresence(), imageFileNames, quoteUri, quoteCid](auto post){
                if (presence)
                    continuePost(imageFileNames, post, quoteUri, quoteCid);
            });
        return;
    }

    postMaster()->checkPostExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, imageFileNames, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid]
        {
            if (!presence)
                return;

            auto replyRef = createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, std::move(replyRef),
                [this, presence, imageFileNames, quoteUri, quoteCid](auto post){
                    if (presence)
                        continuePost(imageFileNames, post, quoteUri, quoteCid);
                });
        },
        [this, presence=getPresence()] (const QString& error){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error;
            emit postFailed(tr("It seems the post you reply to has been deleted."));
        });
}

void PostUtils::post(QString text, const LinkCard* card,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid)
{
    Q_ASSERT(card);
    qDebug() << "Posting:" << text;

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, nullptr,
            [this, presence=getPresence(), card, quoteUri, quoteCid](auto post){
                if (presence)
                    continuePost(card, post, quoteUri, quoteCid);
            });
        return;
    }

    postMaster()->checkPostExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, card, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid]
        {
            if (!presence)
                return;

            auto replyRef = createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, std::move(replyRef),
                [this, presence, card, quoteUri, quoteCid](auto post){
                    if (presence)
                        continuePost(card, post, quoteUri, quoteCid);
                });
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error;
            emit postFailed(tr("It seems the post you reply to has been deleted."));
        });
}

void PostUtils::continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid)
{
    if (quoteUri.isEmpty())
    {
        continuePost(imageFileNames, post);
        return;
    }

    postMaster()->checkPostExists(quoteUri, quoteCid,
        [this, presence=getPresence(), imageFileNames, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePost(imageFileNames, post);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error;
            emit postFailed(tr("It seems the quoted post has been deleted."));
        });
}

void PostUtils::continuePost(const QStringList& imageFileNames, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex)
{
    if (imgIndex >= imageFileNames.size())
    {
        continuePost(post);
        return;
    }

    const auto& fileName = imageFileNames[imgIndex];
    QByteArray blob;
    const QString mimeType = createBlob(blob, fileName);

    if (blob.isEmpty())
    {
        const QString error = tr("Could not load image") + ": " + QFileInfo(fileName).fileName();
        emit postFailed(error);
        return;
    }

    emit postProgress(tr("Uploading image") + QString(" #%1").arg(imgIndex + 1));

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), imageFileNames, post, imgIndex](auto blob){
            if (!presence)
                return;

            postMaster()->addImageToPost(*post, std::move(blob));
            continuePost(imageFileNames, post, imgIndex + 1);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error;
            emit postFailed(error);
        });
}

void PostUtils::continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid)
{
    if (quoteUri.isEmpty())
    {
        continuePost(card, post);
        return;
    }

    postMaster()->checkPostExists(quoteUri, quoteCid,
        [this, presence=getPresence(), card, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePost(card, post);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error;
            emit postFailed(tr("It seems the quoted post has been deleted."));
        });
}

void PostUtils::continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    Q_ASSERT(card);
    if (card->getThumb().isEmpty())
    {
        continuePost(card, QImage(), post);
        return;
    }

    emit postProgress(tr("Retrieving card image"));

    imageReader()->getImage(card->getThumb(),
        [this, presence=getPresence(), card, post](auto image){
            if (presence)
                continuePost(card, image, post);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error;
            emit postFailed(error);
        });
}

void PostUtils::continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    Q_ASSERT(card);
    QByteArray blob;
    QString mimeType;

    if (!thumb.isNull())
        mimeType = createBlob(blob, thumb, card->getThumb());

    if (blob.isEmpty())
    {
        postMaster()->addExternalToPost(*post, card->getLink(), card->getTitle(), card->getDescription());
        continuePost(post);
        return;
    }

    emit postProgress(tr("Uploading card image"));

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), card, post](auto blob){
            if (!presence)
                return;

            postMaster()->addExternalToPost(*post, card->getLink(), card->getTitle(),
                    card->getDescription(), std::move(blob));
            continuePost(post);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error;
            emit postFailed(error);
        });
}

void PostUtils::continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    emit postProgress(tr("Posting"));
    postMaster()->post(*post,
        [this, presence=getPresence(), post]{
            if (post->mReply && post->mReply->mParent)
            {
                mSkywalker->makeLocalModelChange(
                    [post](auto* model){
                        model->updateReplyCountDelta(post->mReply->mParent->mCid, 1);
                    });
            }

            if (presence)
                emit postOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error;
            emit postFailed(error);
        });
}

void PostUtils::repost(const QString& uri, const QString& cid)
{
    emit repostProgress(tr("Reposting"));

    postMaster()->checkPostExists(uri, cid,
        [this, presence=getPresence(), uri, cid]{
            if (presence)
                continueRepost(uri, cid);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error;
            emit repostFailed(error);
        });
}

void PostUtils::continueRepost(const QString& uri, const QString& cid)
{
    postMaster()->repost(uri, cid,
        [this, presence=getPresence(), cid](const auto& repostUri, const auto&){
            mSkywalker->makeLocalModelChange(
                [cid, repostUri](auto* model){
                    model->updateRepostCountDelta(cid, 1);
                    model->updateRepostUri(cid, repostUri);
                });

            if (presence)
                emit repostOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error;
            emit repostFailed(error);
        });
}

void PostUtils::undoRepost(const QString& repostUri, const QString& origPostCid)
{
    postMaster()->undo(repostUri,
        [this, presence=getPresence(), origPostCid]{
            mSkywalker->makeLocalModelChange(
                [origPostCid](auto* model){
                    model->updateRepostCountDelta(origPostCid, -1);
                    model->updateRepostUri(origPostCid, "");
                });

            if (presence)
                emit undoRepostOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Undo repost failed:" << error;
            emit undoRepostFailed(error);
        });
}

void PostUtils::like(const QString& uri, const QString& cid)
{
    postMaster()->like(uri, cid,
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            mSkywalker->makeLocalModelChange(
                [cid, likeUri](auto* model){
                    model->updateLikeCountDelta(cid, 1);
                    model->updateLikeUri(cid, likeUri);
                });

            if (presence)
                emit likeOk();
        },
        [this, presence=getPresence()](const QString& error){
          if (!presence)
              return;

          qDebug() << "Like failed:" << error;
          emit likeFailed(error);
        });
}

void PostUtils::undoLike(const QString& likeUri, const QString& cid)
{
    postMaster()->undo(likeUri,
        [this, presence=getPresence(), cid]{
            mSkywalker->makeLocalModelChange(
                [cid](auto* model){
                    model->updateLikeCountDelta(cid, -1);
                    model->updateLikeUri(cid, "");
                });

            if (presence)
                emit undoLikeOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error;
            emit undoLikeFailed(error);
        });
}

void PostUtils::pickPhoto() const
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

void PostUtils::setFirstPostLink(const QString& link)
{
    if (link == mFirstPostLink)
        return;

    mFirstPostLink = link;
    emit firstPostLinkChanged();
}

QString PostUtils::highlightMentionsAndLinks(const QString& text, const QString& preeditText,
                                             int cursor, const QString& color)
{
    const QString fullText = text.sliced(0, cursor) + preeditText + text.sliced(cursor);

    const auto facets = postMaster()->parseFacets(fullText);

    // Keep all white space as the user is editing plain text.
    // We only use HTML for highlighting links and mentions
    QString highlighted = "<span style=\"white-space: pre-wrap\">";

    int pos = 0;
    bool webLinkFound = false;
    bool postLinkFound = false;
    mLinkShorteningReduction = 0;

    for (const auto& facet : facets)
    {
        if (facet.mType == ATProto::PostMaster::ParsedMatch::Type::LINK)
        {
            const auto atUri = ATProto::ATUri::fromHttpsUri(facet.mMatch);
            if (atUri.isValid())
            {
                if (!postLinkFound)
                {
                    setFirstPostLink(facet.mMatch);
                    postLinkFound = true;
                }
            }
            else if (!webLinkFound)
            {
                setFirstWebLink(facet.mMatch);
                webLinkFound = true;
            }

            const auto shortLink = ATProto::PostMaster::shortenWebLink(facet.mMatch);
            const int reduction = graphemeLength(facet.mMatch) - graphemeLength(shortLink);
            qDebug() << "SHORT:" << shortLink << "reduction:" << reduction;
            mLinkShorteningReduction += reduction;
        }

        const auto before = fullText.sliced(pos, facet.mStartIndex - pos);
        highlighted.append(before.toHtmlEscaped());
        QString highlight = QString("<font color=\"%1\">%2</font>").arg(color, facet.mMatch);
        highlighted.append(highlight);
        pos = facet.mEndIndex;
    }

    if (!webLinkFound)
        setFirstWebLink(QString());

    if (!postLinkFound)
        setFirstPostLink(QString());

    highlighted.append(fullText.sliced(pos).toHtmlEscaped());
    highlighted.append("</span>");
    return highlighted;
}

int PostUtils::graphemeLength(const QString& text) const
{
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    int length = 0;

    while (boundaryFinder.toNextBoundary() != -1)
        ++length;

    return length;
}

void PostUtils::getQuotePost(const QString& httpsUri)
{
    postMaster()->getPost(httpsUri,
        [this](const auto& uri, const auto& cid, auto post, auto author){
            BasicProfile profile(author->mDid,
                                 author->mHandle,
                                 author->mDisplayName.value_or(""),
                                 author->mAvatar.value_or(""));
            emit quotePost(uri, cid, post->mText, profile, post->mCreatedAt);
        });
}

}
