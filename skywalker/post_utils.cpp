// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"
#include "shared_image_provider.h"
#include "skywalker.h"
#include "unicode_fonts.h"
#include <atproto/lib/rich_text_master.h>
#include <QImageReader>

namespace Skywalker {

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

void PostUtils::post(const QString& text, const QStringList& imageFileNames, const QStringList& altTexts,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid,
                     const QStringList& labels)
{
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, nullptr,
            [this, presence=getPresence(), imageFileNames, altTexts, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(imageFileNames, altTexts, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, imageFileNames, altTexts , replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, labels]
        {
            if (!presence)
                return;

            auto replyRef = createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, std::move(replyRef),
                [this, presence, imageFileNames, altTexts, quoteUri, quoteCid, labels](auto post){
                    if (presence)
                        continuePost(imageFileNames, altTexts , post, quoteUri, quoteCid, labels);
                });
        },
        [this, presence=getPresence()] (const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Reply-to post") + ": " + msg);
        });
}

void PostUtils::post(const QString& text, const LinkCard* card,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid,
                     const QStringList& labels)
{
    Q_ASSERT(card);
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, nullptr,
            [this, presence=getPresence(), card, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(card, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, card, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, labels]
        {
            if (!presence)
                return;

            auto replyRef = createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, std::move(replyRef),
                [this, presence, card, quoteUri, quoteCid, labels](auto post){
                    if (presence)
                        continuePost(card, post, quoteUri, quoteCid, labels);
                });
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Reply-to post") + ": " + msg);
        });
}

void PostUtils::addThreadgate(const QString& uri, bool allowMention, bool allowFollowing, const QStringList& allowLists)
{
    qDebug() << "Add threadgate uri:" << uri << "mention:" << allowMention << "following:" << allowFollowing;

    if (!postMaster())
        return;

    mPostMaster->addThreadgate(uri, allowMention, allowFollowing, allowLists,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            emit threadgateOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addThreadgate failed:" << error << " - " << msg;
            emit threadgateFailed(msg);
        });
}

void PostUtils::continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid, const QStringList& labels)
{
    ATProto::PostMaster::addLabelsToPost(*post, labels);

    if (quoteUri.isEmpty())
    {
        continuePost(imageFileNames, altTexts, post);
        return;
    }

    if (!postMaster())
        return;

    postMaster()->checkRecordExists(quoteUri, quoteCid,
        [this, presence=getPresence(), imageFileNames, altTexts, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePost(imageFileNames, altTexts, post);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Quoted post") + ": " + msg);
        });
}

void PostUtils::continuePost(const QStringList& imageFileNames, const QStringList& altTexts, ATProto::AppBskyFeed::Record::Post::SharedPtr post, int imgIndex)
{
    if (imgIndex >= imageFileNames.size())
    {
        continuePost(post);
        return;
    }

    emit postProgress(tr("Uploading image") + QString(" #%1").arg(imgIndex + 1));

    const auto& fileName = imageFileNames[imgIndex];
    QByteArray blob;
    const QString mimeType = createBlob(blob, fileName);

    if (blob.isEmpty())
    {
        emit postFailed(tr("Could not load image") + QString(" #%1").arg(imgIndex + 1));
        return;
    }

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), imageFileNames, altTexts, post, imgIndex](auto blob){
            if (!presence)
                return;

            postMaster()->addImageToPost(*post, std::move(blob), altTexts[imgIndex]);
            continuePost(imageFileNames, altTexts, post, imgIndex + 1);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error << " - " << msg;
            emit postFailed(msg);
        });
}

void PostUtils::continuePost(const LinkCard* card, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid, const QStringList& labels)
{
    ATProto::PostMaster::addLabelsToPost(*post, labels);

    if (quoteUri.isEmpty())
    {
        continuePost(card, post);
        return;
    }

    if (!postMaster())
        return;

    postMaster()->checkRecordExists(quoteUri, quoteCid,
        [this, presence=getPresence(), card, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePost(card, post);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Quoted post") + ": " + msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error << " - " << msg;
            emit postFailed(msg);
        });
}

void PostUtils::continuePost(ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    postMaster()->post(*post,
        [this, presence=getPresence(), post](const QString& uri, const QString& cid){
            if (!presence)
                return;

            if (post->mReply && post->mReply->mParent)
            {
                mSkywalker->makeLocalModelChange(
                    [post](LocalPostModelChanges* model){
                        model->updateReplyCountDelta(post->mReply->mParent->mCid, 1);
                    });
            }

            emit postOk(uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post failed:" << error << " - " << msg;
            emit postFailed(msg);
        });
}

void PostUtils::repost(const QString& uri, const QString& cid)
{
    if (!postMaster())
        return;

    emit repostProgress(tr("Reposting"));

    postMaster()->checkRecordExists(uri, cid,
        [this, presence=getPresence(), uri, cid]{
            if (presence)
                continueRepost(uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg;
            emit repostFailed(msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg;
            emit repostFailed(msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo repost failed:" << error << " - " << msg;
            emit undoRepostFailed(msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg;
            emit likeFailed(msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg;
            emit undoLikeFailed(msg);
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
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "deletePost failed:" << error << " - " << msg;
            emit postDeletedFailed(msg);
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

void PostUtils::savePhoto(const QString& sourceUrl)
{
    ::Skywalker::savePhoto(sourceUrl,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            qDebug() << "Saved photo!";
            mSkywalker->showStatusMessage(tr("Picture saved"), QEnums::STATUS_LEVEL_INFO);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            mSkywalker->showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

QString PostUtils::cutPhotoRect(const QString& source, const QRect& rect, const QSize& scaledSize)
{
    QImage img = ::Skywalker::cutRect(source, rect);

    if (img.isNull())
        return {};

    img = img.scaled(scaledSize);
    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    return imgProvider->addImage(img);
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

void PostUtils::setFirstFeedLink(const QString& link)
{
    if (link == mFirstFeedLink)
        return;

    mFirstFeedLink = link;
    emit firstFeedLinkChanged();
}

void PostUtils::setFirstListLink(const QString& link)
{
    if (link == mFirstListLink)
        return;

    mFirstListLink = link;
    emit firstListLinkChanged();
}

void PostUtils::setHighlightDocument(QQuickTextDocument* doc, const QString& highlightColor,
        int maxLength, const QString& lengthExceededColor)
{
    mFacetHighlighter.setDocument(doc->textDocument());
    mFacetHighlighter.setHighlightColor(highlightColor);
    mFacetHighlighter.setMaxLength(maxLength, lengthExceededColor);
}

void PostUtils::setHighLightMaxLength(int maxLength)
{
    mFacetHighlighter.setMaxLength(maxLength);
}

void PostUtils::extractMentionsAndLinks(const QString& text, const QString& preeditText,
                                        int cursor)
{
    const QString fullText = text.sliced(0, cursor) + preeditText + text.sliced(cursor);
    const auto facets = ATProto::RichTextMaster::parseFacets(fullText);

    int preeditCursor = cursor + preeditText.length();
    bool editMentionFound = false;
    bool webLinkFound = false;
    bool postLinkFound = false;
    bool feedLinkFound = false;
    bool listLinkFound = false;
    mLinkShorteningReduction = 0;

    for (const auto& facet : facets)
    {
        switch (facet.mType)
        {
        case ATProto::RichTextMaster::ParsedMatch::Type::LINK:
        {
            const auto atUri = ATProto::ATUri::fromHttpsPostUri(facet.mMatch);

            if (atUri.isValid())
            {
                qDebug() << "Valid post link:" << facet.mMatch;

                if (!postLinkFound)
                {
                    setFirstPostLink(facet.mMatch);
                    postLinkFound = true;
                }
            }
            else
            {
                const auto atFeedUri = ATProto::ATUri::fromHttpsFeedUri(facet.mMatch);

                if (atFeedUri.isValid())
                {
                    qDebug() << "Valid feed link:" << facet.mMatch;

                    if (!feedLinkFound)
                    {
                        setFirstFeedLink(facet.mMatch);
                        feedLinkFound = true;
                    }
                }
                else
                {
                    const auto atListUri = ATProto::ATUri::fromHttpsListUri(facet.mMatch);

                    if (atListUri.isValid())
                    {
                        qDebug() << "Valid list link:" << facet.mMatch;

                        if (!listLinkFound)
                        {
                            setFirstListLink(facet.mMatch);
                            listLinkFound = true;
                        }
                    }
                    else if (!webLinkFound)
                    {
                        qDebug() << "Web link:" << facet.mMatch;

                        setFirstWebLink(facet.mMatch);
                        webLinkFound = true;
                    }
                }
            }

            const auto shortLink = ATProto::RichTextMaster::shortenWebLink(facet.mMatch);
            const int reduction = UnicodeFonts::graphemeLength(facet.mMatch) - UnicodeFonts::graphemeLength(shortLink);
            qDebug() << "SHORT:" << shortLink << "reduction:" << reduction;
            mLinkShorteningReduction += reduction;
            break;
        }
        case ATProto::RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::MENTION:
            if (facet.mStartIndex < preeditCursor && preeditCursor <= facet.mEndIndex)
            {
                mEditMentionIndex = facet.mStartIndex + 1;
                setEditMention(facet.mMatch.sliced(1)); // strip @-symbol
                editMentionFound = true;
            }
            break;
        case ATProto::RichTextMaster::ParsedMatch::Type::TAG:
        case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }

    if (!editMentionFound)
        setEditMention({});

    if (!webLinkFound)
        setFirstWebLink({});

    if (!postLinkFound)
        setFirstPostLink({});

    if (!feedLinkFound)
        setFirstFeedLink({});

    if (!listLinkFound)
        setFirstListLink({});
}

QString PostUtils::applyFontToLastTypedChars(const QString& text,const QString& preeditText,
                                             int cursor, int numChars, QEnums::FontType font)
{
    if (font == QEnums::FONT_NORMAL)
        return {};

    if (!mEditMention.isEmpty())
    {
        qDebug() << "Editing a mention";
        return {};
    }

    QString modifiedText = text.sliced(0, cursor) + preeditText;

    if (UnicodeFonts::convertLastCharsToFont(modifiedText, numChars, font))
        return modifiedText;

    return {};
}

QString PostUtils::linkiFy(const QString& text, const QString& colorName)
{
    return ATProto::RichTextMaster::linkiFy(text, colorName);
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

void PostUtils::getQuoteFeed(const QString& httpsUri)
{
    if (!postMaster())
        return;

    postMaster()->getFeed(httpsUri,
        [this](auto feed){
            ATProto::AppBskyFeed::GeneratorView::SharedPtr sharedFeed(feed.release());
            GeneratorView view(sharedFeed);
            emit quoteFeed(view);
        });
}

void PostUtils::getQuoteList(const QString& httpsUri)
{
    if (!postMaster())
        return;

    postMaster()->getList(httpsUri,
                          [this](auto list){
                              ATProto::AppBskyGraph::ListView::SharedPtr sharedList(list.release());
                              ListView view(sharedList);
                              emit quoteList(view);
                          });
}



void PostUtils::sharePhoto(int fd)
{
    if (!mPickingPhoto)
        return;

    mPickingPhoto = false;
    qDebug() << "Share photo fd:" << fd;
    auto [img, error] = readImageFd(fd);

    if (img.isNull())
    {
        emit photoPickFailed(error);
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
