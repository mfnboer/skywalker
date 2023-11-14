// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"
#include "shared_image_provider.h"
#include <QImageReader>

namespace Skywalker {

QString PostUtils::toPlainText(const QString& text)
{
    QTextDocument doc;
    doc.setHtml(text);
    return doc.toPlainText();
}

PostUtils::PostUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::photoPicked,
            this, [this](int fd){ sharePhoto(fd);});

    connect(&jniCallbackListener, &JNICallbackListener::photoPickCanceled,
            this, [this]{ cancelPhotoPicking(); });

    connect(this, &WrappedSkywalker::skywalkerChanged, this, [this]{
        if (!mSkywalker)
            return;

        connect(mSkywalker, &Skywalker::bskyClientDeleted, this,
            [this]{
                qDebug() << "Reset post master";
                mPostMaster = nullptr;
            });
    });
}

ATProto::PostMaster* PostUtils::postMaster()
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

ImageReader* PostUtils::imageReader()
{
    if (!mImageReader)
        mImageReader = std::make_unique<ImageReader>();

    return mImageReader.get();
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

void PostUtils::post(QString text, const QStringList& imageFileNames, const QStringList& altTexts,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid)
{
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, nullptr,
            [this, presence=getPresence(), imageFileNames, altTexts, quoteUri, quoteCid](auto post){
                if (presence)
                    continuePost(imageFileNames, altTexts, post, quoteUri, quoteCid);
            });
        return;
    }

    postMaster()->checkPostExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, imageFileNames, altTexts , replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid]
        {
            if (!presence)
                return;

            auto replyRef = createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, std::move(replyRef),
                [this, presence, imageFileNames, altTexts, quoteUri, quoteCid](auto post){
                    if (presence)
                        continuePost(imageFileNames, altTexts , post, quoteUri, quoteCid);
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

    if (!postMaster())
        return;

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

void PostUtils::continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid)
{
    if (quoteUri.isEmpty())
    {
        continuePost(imageFileNames, altTexts, post);
        return;
    }

    if (!postMaster())
        return;

    postMaster()->checkPostExists(quoteUri, quoteCid,
        [this, presence=getPresence(), imageFileNames, altTexts, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePost(imageFileNames, altTexts, post);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error;
            emit postFailed(tr("It seems the quoted post has been deleted."));
        });
}

void PostUtils::continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex)
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
        [this, presence=getPresence(), imageFileNames, altTexts, post, imgIndex](auto blob){
            if (!presence)
                return;

            postMaster()->addImageToPost(*post, std::move(blob), altTexts[imgIndex]);
            continuePost(imageFileNames, altTexts, post, imgIndex + 1);
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

    if (!postMaster())
        return;

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
    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    postMaster()->post(*post,
        [this, presence=getPresence(), post]{
            if (!presence)
                return;

            if (post->mReply && post->mReply->mParent)
            {
                mSkywalker->makeLocalModelChange(
                    [post](LocalPostModelChanges* model){
                        model->updateReplyCountDelta(post->mReply->mParent->mCid, 1);
                    });
            }

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
    if (!postMaster())
        return;

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
    if (!postMaster())
        return;

    postMaster()->repost(uri, cid,
        [this, presence=getPresence(), cid](const auto& repostUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, repostUri](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(cid, 1);
                    model->updateRepostUri(cid, repostUri);
                });

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
    if (!postMaster())
        return;

    postMaster()->undo(repostUri,
        [this, presence=getPresence(), origPostCid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [origPostCid](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(origPostCid, -1);
                    model->updateRepostUri(origPostCid, "");
                });

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
    if (!postMaster())
        return;

    postMaster()->like(uri, cid,
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, likeUri](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, 1);
                    model->updateLikeUri(cid, likeUri);
                });

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
    if (!postMaster())
        return;

    postMaster()->undo(likeUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, -1);
                    model->updateLikeUri(cid, "");
                });

            emit undoLikeOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error;
            emit undoLikeFailed(error);
        });
}

void PostUtils::deletePost(const QString& postUri, const QString& cid)
{
    if (!postMaster())
        return;

    postMaster()->undo(postUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updatePostDeleted(cid);
                });

            emit postDeletedOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "deletePost failed:" << error;
            emit postDeletedFailed(error);
        });
}

bool PostUtils::pickPhoto()
{
    const bool permission = ::Skywalker::pickPhoto();

    if (!permission)
    {
        mSkywalker->showStatusMessage(
            tr("No permission to pick photo. Try sharing a photo from your gallery app."),
            QEnums::STATUS_LEVEL_ERROR);
    }
    else
    {
        mPickingPhoto = true;
    }

    return permission;
}

void PostUtils::setEditMention(const QString& mention)
{
    if (mention == mEditMention)
        return;

    mEditMention = mention;
    emit editMentionChanged();
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

void PostUtils::setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor)
{
    mFacetHighlighter.setDocument(doc->textDocument());
    mFacetHighlighter.setHighlightColor(highlightColor);
}

void PostUtils::extractMentionsAndLinks(const QString& text, const QString& preeditText,
                                        int cursor)
{   
    const QString fullText = text.sliced(0, cursor) + preeditText + text.sliced(cursor);
    const auto facets = postMaster()->parseFacets(fullText);

    int preeditCursor = cursor + preeditText.length();
    bool editMentionFound = false;
    bool webLinkFound = false;
    bool postLinkFound = false;
    mLinkShorteningReduction = 0;

    for (const auto& facet : facets)
    {
        switch (facet.mType)
        {
        case ATProto::PostMaster::ParsedMatch::Type::LINK:
        {
            const auto atUri = ATProto::ATUri::fromHttpsPostUri(facet.mMatch);
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
            break;
        }
        case ATProto::PostMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::PostMaster::ParsedMatch::Type::MENTION:
            if (facet.mStartIndex < preeditCursor && preeditCursor <= facet.mEndIndex)
            {
                mEditMentionIndex = facet.mStartIndex + 1;
                setEditMention(facet.mMatch.sliced(1)); // strip @-symbol
                editMentionFound = true;
            }
            break;
        case ATProto::PostMaster::ParsedMatch::Type::TAG:
        case ATProto::PostMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }

    if (!editMentionFound)
        setEditMention(QString());

    if (!webLinkFound)
        setFirstWebLink(QString());

    if (!postLinkFound)
        setFirstPostLink(QString());
}

QString PostUtils::linkiFy(const QString& text, const QString& colorName)
{
    const auto facets = postMaster()->parseFacets(text);
    QString linkified = "<span style=\"white-space: pre-wrap\">";

    int pos = 0;

    for (const auto& facet : facets)
    {
        if (facet.mType == ATProto::PostMaster::ParsedMatch::Type::MENTION ||
            facet.mType == ATProto::PostMaster::ParsedMatch::Type::LINK)
        {
            const auto before = text.sliced(pos, facet.mStartIndex - pos);
            linkified.append(before.toHtmlEscaped());
            const QString ref = facet.mType == ATProto::PostMaster::ParsedMatch::Type::MENTION || facet.mMatch.startsWith("http") ?
                                    facet.mMatch : "https://" + facet.mMatch;
            QString link = QString("<a href=\"%1\" style=\"color: %3;\">%2</a>").arg(ref, facet.mMatch, colorName);
            linkified.append(link);
            pos = facet.mEndIndex;
        }
    }

    linkified.append(text.sliced(pos).toHtmlEscaped());
    linkified.append("</span>");
    return linkified;
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
    if (!postMaster())
        return;

    postMaster()->getPost(httpsUri,
        [this](const auto& uri, const auto& cid, auto post, auto author){
            BasicProfile profile(author->mDid,
                                 author->mHandle,
                                 author->mDisplayName.value_or(""),
                                 author->mAvatar.value_or(""));
            emit quotePost(uri, cid, post->mText, profile, post->mCreatedAt);
        });
}

bool PostUtils::onlyEmojis(const QString& text)
{
    for (const auto c : text.toUcs4())
    {
        if (!isEmoji(c))
            return false;
    }

    return true;
}

bool PostUtils::isEmoji(uint c)
{
    static const std::map<uint, uint> RANGES = {
        {0x1F600, 0x1F64F},
        {0x1F300, 0x1F5FF},
        {0x1F680, 0x1F6C5},
        {0x1F700, 0x1FAFF}
    };

    auto it = RANGES.upper_bound(c);

    if (it == RANGES.begin())
        return false;

    --it;

    return c >= it->first && c <= it->second;
}

void PostUtils::sharePhoto(int fd)
{
    if (!mPickingPhoto)
        return;

    mPickingPhoto = false;
    qDebug() << "Share photo fd:" << fd;

    if (fd < 0)
    {
        emit photoPickFailed(tr("Could not open picture."));
        return;
    }

    QFile file;

    if (!file.open(fd, QFile::OpenModeFlag::ReadOnly, QFile::FileHandleFlag::AutoCloseHandle))
    {
        qWarning() << "Could not open file";
        emit photoPickFailed("Could not open picture file.");
        return;
    }

    QImageReader reader(&file);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
    {
        qWarning() << "Could not read picture data.";
        emit photoPickFailed(tr("Could not read picture data."));
        return;
    }

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString source = imgProvider->addImage(img);
    emit photoPicked(source);
}

void PostUtils::cancelPhotoPicking()
{
    if (!mPickingPhoto)
        return;

    mPickingPhoto = false;
    qDebug() << "Cancel photo picking";
    emit photoPickCanceled();
}

void PostUtils::dropPhoto(const QString& source)
{
    if (source.startsWith("image://"))
    {
        auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        imgProvider->removeImage(source);
    }
}

}
