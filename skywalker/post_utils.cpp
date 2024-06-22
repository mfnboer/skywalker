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

void PostUtils::post(const QString& text, const QStringList& imageFileNames, const QStringList& altTexts,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid,
                     const QStringList& labels, const QString& language)
{
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, language, nullptr,
            [this, presence=getPresence(), imageFileNames, altTexts, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(imageFileNames, altTexts, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, imageFileNames, altTexts , replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, labels, language]
        {
            if (!presence)
                return;

            auto replyRef = ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, language, std::move(replyRef),
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
                     const QStringList& labels, const QString& language)
{
    Q_ASSERT(card);
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        postMaster()->createPost(text, language, nullptr,
            [this, presence=getPresence(), card, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(card, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, card, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, labels, language]
        {
            if (!presence)
                return;

            auto replyRef = ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

            postMaster()->createPost(text, language, std::move(replyRef),
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

void PostUtils::addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollowing, const QStringList& allowList)
{
    qDebug() << "Add threadgate uri:" << uri << "mention:" << allowMention << "following:" << allowFollowing;

    if (!postMaster())
        return;

    mPostMaster->addThreadgate(uri, allowMention, allowFollowing, allowList,
        [this, presence=getPresence(), cid, allowMention, allowFollowing, allowList](const QString& threadgateUri, const QString&){
            if (!presence)
                return;

            ListViewBasicList restrictionLists;
            for (const auto& uri : allowList)
                restrictionLists.push_back(ListViewBasic(uri, "", uri, ATProto::AppBskyGraph::ListPurpose::CURATE_LIST, ""));

            mSkywalker->makeLocalModelChange(
                [cid, threadgateUri, allowMention, allowFollowing, allowList, restrictionLists](LocalPostModelChanges* model){
                    model->updateThreadgateUri(cid, threadgateUri);
                    model->updateReplyRestriction(cid, Post::makeReplyRestriction(allowMention, allowFollowing, !allowList.empty()));
                    model->updateReplyRestrictionLists(cid, restrictionLists);
                });

            emit threadgateOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addThreadgate failed:" << error << " - " << msg;
            emit threadgateFailed(msg);
        });
}

void PostUtils::undoThreadgate(const QString& threadgateUri, const QString& cid)
{
    if (!postMaster())
        return;

    postMaster()->undo(threadgateUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateThreadgateUri(cid, "");
                    model->updateReplyRestriction(cid, QEnums::REPLY_RESTRICTION_NONE);
                    model->updateReplyRestrictionLists(cid, {});
                });

            emit undoThreadgateOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo threadgate failed:" << error << " - " << msg;
            emit undoThreadgateFailed(msg);
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

    emit postProgress(tr("Uploading image #%1").arg(imgIndex + 1));

    const auto& fileName = imageFileNames[imgIndex];
    QByteArray blob;
    const QString mimeType = PhotoPicker::createBlob(blob, fileName);

    if (blob.isEmpty())
    {
        emit postFailed(tr("Could not load image #%1").arg(imgIndex + 1));
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
        mimeType = PhotoPicker::createBlob(blob, thumb, card->getThumb());

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

void PostUtils::batchDeletePosts(const QStringList& postUris)
{
    if (!bskyClient())
        return;

    ATProto::ComATProtoRepo::ApplyWritesList writes;

    for (const auto& uri : postUris)
    {
        const ATProto::ATUri atUri(uri);

        if (!atUri.isValid())
        {
            qWarning() << "Invalid post URI:" << uri;
            continue;
        }

        auto deleteRecord = std::make_unique<ATProto::ComATProtoRepo::ApplyWritesDelete>();
        deleteRecord->mCollection = atUri.getCollection();
        deleteRecord->mRKey = atUri.getRkey();
        writes.push_back(std::move(deleteRecord));
    }

    const QString& repo = mSkywalker->getUserDid();

    bskyClient()->applyWrites(repo, writes, false,
        []{
            qDebug() << "Deleted posts";
        },
        [](const QString& error, const QString& msg) {
            qWarning() << "Failed to delete posts:" << error << "-" << msg;
        });
}

bool PostUtils::pickPhoto()
{
    const bool permission = PhotoPicker::pickPhoto();

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
    PhotoPicker::savePhoto(sourceUrl,
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
    QImage img = PhotoPicker::cutRect(source, rect);

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

void PostUtils::setEditTag(const QString& tag)
{
    if (tag == mEditTag)
        return;

    mEditTag = tag;
    emit editTagChanged();
}

void PostUtils::setFirstWebLink(const QString& link)
{
    if (link != mFirstWebLink)
    {
        mFirstWebLink = link;
        emit firstWebLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstWebLink(false);
}

void PostUtils::setFirstWebLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstWebLink(linkMatch.mMatch);
    const bool inLink = cursor >= linkMatch.mStartIndex && cursor <= linkMatch.mEndIndex;
    setCursorInFirstWebLink(inLink);
}

void PostUtils::setCursorInFirstWebLink(bool inLink)
{
    if (inLink == mCursorInFirstWebLink)
        return;

    mCursorInFirstWebLink = inLink;
    emit cursorInFirstWebLinkChanged();
}

void PostUtils::setFirstPostLink(const QString& link)
{
    if (link != mFirstPostLink)
    {
        mFirstPostLink = link;
        emit firstPostLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstPostLink(false);
}

void PostUtils::setFirstPostLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstPostLink(linkMatch.mMatch);
    const bool inLink = cursor >= linkMatch.mStartIndex && cursor <= linkMatch.mEndIndex;
    setCursorInFirstPostLink(inLink);
}

void PostUtils::setCursorInFirstPostLink(bool inLink)
{
    if (inLink == mCursorInFirstPostLink)
        return;

    mCursorInFirstPostLink = inLink;
    emit cursorInFirstPostLinkChanged();
}

void PostUtils::setFirstFeedLink(const QString& link)
{
    if (link != mFirstFeedLink)
    {
        mFirstFeedLink = link;
        emit firstFeedLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstFeedLink(false);
}

void PostUtils::setFirstFeedLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstFeedLink(linkMatch.mMatch);
    const bool inLink = cursor >= linkMatch.mStartIndex && cursor <= linkMatch.mEndIndex;
    setCursorInFirstFeedLink(inLink);
}

void PostUtils::setCursorInFirstFeedLink(bool inLink)
{
    if (inLink == mCursorInFirstFeedLink)
        return;

    mCursorInFirstFeedLink = inLink;
    emit cursorInFirstFeedLinkChanged();
}

void PostUtils::setFirstListLink(const QString& link)
{
    if (link != mFirstListLink)
    {
        mFirstListLink = link;
        emit firstListLinkChanged();
    }

    if (link.isEmpty())
        setCursorInFirstListLink(false);
}

void PostUtils::setFirstListLink(const ATProto::RichTextMaster::ParsedMatch& linkMatch, int cursor)
{
    setFirstListLink(linkMatch.mMatch);
    const bool inLink = cursor >= linkMatch.mStartIndex && cursor <= linkMatch.mEndIndex;
    setCursorInFirstListLink(inLink);
}

void PostUtils::setCursorInFirstListLink(bool inLink)
{
    if (inLink == mCursorInFirstListLink)
        return;

    mCursorInFirstListLink = inLink;
    emit cursorInFirstListLinkChanged();
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
    bool editTagFound = false;
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
                    setFirstPostLink(facet, cursor);
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
                        setFirstFeedLink(facet, cursor);
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
                            setFirstListLink(facet, cursor);
                            listLinkFound = true;
                        }
                    }
                    else if (!webLinkFound)
                    {
                        qDebug() << "Web link:" << facet.mMatch;

                        setFirstWebLink(facet, cursor);
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
            if (facet.mStartIndex < preeditCursor && preeditCursor <= facet.mEndIndex)
            {
                mEditTagIndex = facet.mStartIndex + 1;
                setEditTag(facet.mMatch.sliced(1)); // strip #-symbol
                editTagFound = true;
            }
            break;
        case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }

    if (!editMentionFound)
        setEditMention({});

    if (!editTagFound)
        setEditTag({});

    if (!webLinkFound)
        setFirstWebLink({});

    if (!postLinkFound)
        setFirstPostLink({});

    if (!feedLinkFound)
        setFirstFeedLink({});

    if (!listLinkFound)
        setFirstListLink({});
}

void PostUtils::cacheTags(const QString& text)
{
    const auto facets = ATProto::RichTextMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        if (facet.mType == ATProto::RichTextMaster::ParsedMatch::Type::TAG)
        {
            auto& hashtags = mSkywalker->getUserHashtags();
            hashtags.insert(facet.mMatch.sliced(1)); // strip #-symbol
        }
    }
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
            const auto formattedText = ATProto::RichTextMaster::getFormattedPostText(*post, UserSettings::getLinkColor());
            emit quotePost(uri, cid, formattedText, profile, post->mCreatedAt);
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
    auto [img, error] = PhotoPicker::readImageFd(fd);

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
