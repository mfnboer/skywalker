// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"
#include "file_utils.h"
#include "jni_callback.h"
#include "photo_picker.h"
#include "shared_image_provider.h"
#include "skywalker.h"
#include "temp_file_holder.h"
#include <atproto/lib/rich_text_master.h>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMimeType>

namespace Skywalker {

static constexpr int MIN_LANGUAGE_IDENTIFICATION_LENGTH = 20;

int PostUtils::sNextRequestId = 1;

PostUtils::PostUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence(),
    mNetwork(new QNetworkAccessManager(this))
{
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);

    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::photoPicked,
        this, [this](int fd, QString mimeType){ shareMedia(fd, mimeType);});

    connect(&jniCallbackListener, &JNICallbackListener::photoPickCanceled,
        this, [this]{ cancelPhotoPicking(); });

    connect(&jniCallbackListener, &JNICallbackListener::languageIdentified,
            this, [this](QString languageCode, int requestId){ handleLanguageIdentified(languageCode, requestId); });

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
        mImageReader = std::make_unique<ImageReader>(mNetwork, this);

    return mImageReader.get();
}

bool PostUtils::isPostUri(const QString& uri)
{
    const ATProto::ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid at-uri:" << uri;
        return false;
    }

    return atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_POST;
}

QString PostUtils::extractDidFromUri(const QString& uri)
{
    const ATProto::ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid at-uri:" << uri;
        return {};
    }

    if (atUri.authorityIsHandle())
    {
        qWarning() << "Authority is not a DID:" << uri;
        return {};
    }

    return atUri.getAuthority();
}

void PostUtils::checkPostExists(const QString& uri, const QString& cid)
{
    qDebug() << "Check post exists, uri:" << uri << "cid:" << cid;

    if (!postMaster())
        return;

    postMaster()->checkRecordExists(uri, cid,
        [this, presence=getPresence(), uri, cid]{
            if (!presence)
                return;

            emit checkPostExistsOk(uri, cid);
        },
        [this, presence=getPresence(), uri, cid] (const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Check post exists failed:" << error << " - " << msg;

            if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
                emit checkPostExistsFailed(uri, cid, tr("Post is deleted"));
            else
                emit checkPostExistsFailed(uri, cid, msg);
        });
}

void PostUtils::canQuotePost(const QString& postUri)
{
    qDebug() << "Check if post can be quoted:" << postUri;

    if (!postMaster())
        return;

    postMaster()->getPostgate(postUri,
        [this, presence=getPresence(), postUri](auto postgate){
            if (!presence)
                return;

            emit canQuotePostOk(postUri, !postgate->mDisableEmbedding);
        },
        [this, presence=getPresence(), postUri](const QString& error, const QString& msg){
            if (!presence)
                return;

            if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
            {
                qDebug() << "No postgate record exists:" << postUri;
                emit canQuotePostOk(postUri, true);
                return;
            }

            qWarning() << "Failed to get postgate:" << error << "-" << msg;
            emit canQuotePostFailed(postUri, msg);
        });
}

void PostUtils::post(const QString& text, const QStringList& imageFileNames, const QStringList& altTexts,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid,
                     const WebLink::List& embeddedLinks,
                     const QStringList& labels, const QString& language)
{
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);
        postMaster()->createPost(text, language, nullptr, embeddedFacets,
            [this, presence=getPresence(), imageFileNames, altTexts, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(imageFileNames, altTexts, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, imageFileNames, altTexts , replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, embeddedLinks, labels, language]
        {
            if (!presence)
                return;

            auto replyRef = ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);
            const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);

            if (!postMaster())
                return;

            postMaster()->createPost(text, language, std::move(replyRef), embeddedFacets,
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
                     const WebLink::List& embeddedLinks,
                     const QStringList& labels, const QString& language)
{
    Q_ASSERT(card);
    qDebug() << "Posting:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    if (replyToUri.isEmpty())
    {
        const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);
        postMaster()->createPost(text, language, nullptr, embeddedFacets,
            [this, presence=getPresence(), card, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePost(card, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, card, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, embeddedLinks, labels, language]
        {
            if (!presence)
                return;

            auto replyRef = ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);
            const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);

            if (!postMaster())
                return;

            postMaster()->createPost(text, language, std::move(replyRef), embeddedFacets,
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

void PostUtils::postVideo(const QString& text, const QString& videoFileName,
                     const QString& videoAltText, int videoWidth, int videoHeight,
                     const QString& replyToUri, const QString& replyToCid,
                     const QString& replyRootUri, const QString& replyRootCid,
                     const QString& quoteUri, const QString& quoteCid,
                     const WebLink::List& embeddedLinks,
                     const QStringList& labels, const QString& language)
{
    qDebug() << "Posting video:" << text;

    if (!postMaster())
        return;

    emit postProgress(tr("Posting"));

    // TODO: code duplication
    if (replyToUri.isEmpty())
    {
        const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);
        postMaster()->createPost(text, language, nullptr, embeddedFacets,
            [this, presence=getPresence(), videoFileName, videoAltText, videoWidth, videoHeight, quoteUri, quoteCid, labels](auto post){
                if (presence)
                    continuePostVideo(videoFileName, videoAltText, videoWidth, videoHeight, post, quoteUri, quoteCid, labels);
            });
        return;
    }

    postMaster()->checkRecordExists(replyToUri, replyToCid,
        [this, presence=getPresence(), text, videoFileName, videoAltText, videoWidth, videoHeight, replyToUri, replyToCid, replyRootUri, replyRootCid, quoteUri, quoteCid, embeddedLinks, labels, language]
        {
            if (!presence)
                return;

            auto replyRef = ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);
            const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);

            if (!postMaster())
                return;

            postMaster()->createPost(text, language, std::move(replyRef), embeddedFacets,
                [this, presence, videoFileName, videoAltText, videoWidth, videoHeight, quoteUri, quoteCid, labels](auto post){
                    if (presence)
                        continuePostVideo(videoFileName, videoAltText, videoWidth, videoHeight, post, quoteUri, quoteCid, labels);
                });
        },
        [this, presence=getPresence()] (const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Reply-to post") + ": " + msg);
        });
}

void PostUtils::addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const QStringList& allowList, bool allowNobody, const QStringList& hiddenReplies)
{
    ListViewBasicList restrictionLists;

    for (const auto& uri : allowList)
        restrictionLists.push_back(ListViewBasic(uri, "", uri, ATProto::AppBskyGraph::ListPurpose::CURATE_LIST, ""));

    addThreadgate(uri, cid, allowMention, allowFollower, allowFollowing, restrictionLists, allowNobody, hiddenReplies);
}

void PostUtils::addThreadgate(const QString& uri, const QString& cid, bool allowMention, bool allowFollower, bool allowFollowing, const ListViewBasicList& allowList, bool allowNobody, const QStringList& hiddenReplies)
{
    qDebug() << "Add threadgate uri:" << uri << "mention:" << allowMention << "follower:" << allowFollower << "following:" << allowFollowing << "nobody:" << allowNobody << "hiddenReplies:" << hiddenReplies.size();

    if (!postMaster())
        return;

    QStringList allowListUris;

    for (const auto& list : allowList)
        allowListUris.push_back(list.getUri());

    postMaster()->addThreadgate(uri, allowMention, allowFollower, allowFollowing, allowListUris, allowNobody, hiddenReplies,
        [this, presence=getPresence(), cid, allowMention, allowFollower, allowFollowing, allowList, allowNobody, hiddenReplies](const QString& threadgateUri, const QString&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, threadgateUri, allowMention, allowFollower, allowFollowing, allowList, allowNobody, hiddenReplies](LocalPostModelChanges* model){
                    model->updateThreadgateUri(cid, threadgateUri);
                    model->updateReplyRestriction(cid, Post::makeReplyRestriction(allowMention, allowFollower, allowFollowing, !allowList.empty(), allowNobody));
                    model->updateReplyRestrictionLists(cid, allowList);
                    model->updateHiddenReplies(cid, hiddenReplies);
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

void PostUtils::addPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris)
{
    qDebug() << "Add postgate uri:" << uri << "disableEmbedding:" << disableEmbedding;

    if (!postMaster())
        return;

    postMaster()->addPostgate(uri, disableEmbedding, detachedEmbeddingUris,
        [this, presence=getPresence()](const QString&, const QString&){
            if (!presence)
                return;

            emit postgateOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addPostgate failed:" << error << " - " << msg;
            emit postgateFailed(msg);
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
                    model->updateHiddenReplies(cid, {});
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

void PostUtils::undoPostgate(const QString& postUri)
{
    qDebug() << "Undo postgate:" << postUri;

    if (!postMaster())
        return;

    const QString postgateUri = postMaster()->createPostgateUri(postUri);
    Q_ASSERT(!postgateUri.isEmpty());

    postMaster()->undo(postgateUri,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            emit undoPostgateOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo postgate failed:" << error << " - " << msg;
            emit undoPostgateFailed(msg);
        });
}

void PostUtils::detachQuote(const QString& postUri, const QString& embeddingUri, const QString& embeddingCid, bool detach)
{
    qDebug() << "Detach quote:" << detach << postUri << "embeddingUri:" << embeddingUri << "embeddingCid:" << embeddingCid;

    if (!postMaster())
        return;

    postMaster()->detachEmbedding(postUri, embeddingUri, embeddingCid, detach,
        [this, postUri, presence=getPresence()](const QString& uri, const QString& cid, bool detached){
            if (!presence)
                return;

            // The returned (uri, cid) is the embedding post
            qDebug() << "Detach quote succeeded:" << uri << cid << detached;
            bool mustLoadReAttachedRecord = false;

            mSkywalker->makeLocalModelChange(
                [postUri, cid, detached, &mustLoadReAttachedRecord](LocalPostModelChanges* model){
                    if (detached)
                    {
                        model->updateDetachedRecord(cid, postUri);
                    }
                    else
                    {
                        if (model->updateDetachedRecord(cid, {}))
                            mustLoadReAttachedRecord = true;
                    }
                });

            if (mustLoadReAttachedRecord)
            {
                // Give network some time to process the postgate change
                // NOTE: if this is not good enough we can fetch the quote itself instead of
                // the embedding post. But then we need to create a RecordView from that.
                QTimer::singleShot(300, this, [this, uri]{ continueReAttachQuote(uri); });
            }
            else
            {
                emit detachQuoteOk(detached);
            }
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Detach quote failed:" << error << " - " << msg;
            emit detachQuoteFailed(msg);
        });
}

void PostUtils::continueReAttachQuote(const QString& embeddingUri, int retries)
{
    qDebug() << "Load for re-attachment:" << embeddingUri << "retries:" << retries;

    if (!bskyClient())
        return;

    bskyClient()->getPosts({embeddingUri},
        [this, presence=getPresence(), embeddingUri, retries](auto postViewList){
            if (!presence)
                return;

            if (postViewList.size() != 1)
            {
                qWarning() << "Wrong list size:" << postViewList.size() << "for uri:" << embeddingUri;
                emit detachQuoteFailed("Could not load quote.");
                return;
            }

            const auto& embed = postViewList[0]->mEmbed;

            if (!embed)
            {
                qWarning() << "Embedded record missing";
                emit detachQuoteFailed("Could not load quote.");
                return;
            }

            RecordView::SharedPtr recordView;

            switch (embed->mType)
            {
            case ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW:
            {
                auto protoRecordView = std::get<ATProto::AppBskyEmbed::RecordView::SharedPtr>(embed->mEmbed);
                recordView = std::make_shared<RecordView>(*protoRecordView);
                break;
            }
            case ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW:
            {
                auto protoRecordWithMediaView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(embed->mEmbed);
                recordView = std::make_shared<RecordView>(*protoRecordWithMediaView->mRecord);
                break;
            }
            default:
                qWarning() << "Unexpected embed type:" << (int)embed->mType;
                emit detachQuoteFailed("Could not load quote.");
                return;
            }

            if (recordView->getDetached())
            {
                if (retries > 0)
                {
                    qDebug() << "Quote is still detached, retry";
                    QTimer::singleShot(500, this, [this, embeddingUri, retries]{ continueReAttachQuote(embeddingUri, retries - 1); });
                    return;
                }
                else
                {
                    qWarning() << "Quote is still detached";
                    emit detachQuoteFailed("Could not load quote.");
                }
            }

            mSkywalker->makeLocalModelChange(
                [cid=postViewList[0]->mCid, recordView](LocalPostModelChanges* model){
                    model->updateReAttachedRecord(cid, recordView);
                });

            emit detachQuoteOk(false);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Detach quote failed:" << error << " - " << msg;
            emit detachQuoteFailed(msg);
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

            if (!postMaster())
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
    const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, fileName);

    if (blob.isEmpty())
    {
        emit postFailed(tr("Could not load image #%1").arg(imgIndex + 1));
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), imgSize, imageFileNames, altTexts, post, imgIndex](auto blob){
            if (!presence)
                return;

            if (!postMaster())
                return;

            postMaster()->addImageToPost(*post, std::move(blob), imgSize.width(), imgSize.height(), altTexts[imgIndex]);
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

            if (!postMaster())
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
        [this, presence=getPresence(), card, post](const QString& error){
            if (!presence)
                return;

            qDebug() << "Failed to load image:" << error;
            // Post the card without image anyway. Sometimes card information at sites
            // have broken image links.
            continuePost(card, QImage(), post);
        });
}

void PostUtils::continuePost(const LinkCard* card, QImage thumb, ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    Q_ASSERT(card);
    QByteArray blob;
    QString mimeType;

    if (!thumb.isNull())
    {
        const auto [imgMime, imgSize] = PhotoPicker::createBlob(blob, thumb, card->getThumb());
        mimeType = imgMime;
    }

    if (blob.isEmpty())
    {
        if (!postMaster())
            return;

        postMaster()->addExternalToPost(*post, card->getLink(), card->getTitle(), card->getDescription());
        continuePost(post);
        return;
    }

    emit postProgress(tr("Uploading card image"));

    if (!bskyClient())
        return;

    bskyClient()->uploadBlob(blob, mimeType,
        [this, presence=getPresence(), card, post](auto blob){
            if (!presence)
                return;

            if (!postMaster())
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

void PostUtils::continuePostVideo(const QString& videoFileName, const QString& videoAltText,
                             int videoWidth, int videoHeight, ATProto::AppBskyFeed::Record::Post::SharedPtr post,
                             const QString& quoteUri, const QString& quoteCid, const QStringList& labels)
{
    ATProto::PostMaster::addLabelsToPost(*post, labels);

    if (quoteUri.isEmpty())
    {
        continuePostVideo(videoFileName, videoAltText, videoWidth, videoHeight, post);
        return;
    }

    if (!postMaster())
        return;

    postMaster()->checkRecordExists(quoteUri, quoteCid,
        [this, presence=getPresence(), videoFileName, videoAltText, videoWidth, videoHeight, post, quoteUri, quoteCid]{
            if (!presence)
                return;

            if (!postMaster())
                return;

            postMaster()->addQuoteToPost(*post, quoteUri, quoteCid);
            continuePostVideo(videoFileName, videoAltText, videoWidth, videoHeight, post);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Post not found:" << error << " - " << msg;
            emit postFailed(tr("Quoted post") + ": " + msg);
        });
}

void PostUtils::continuePostVideo(const QString& videoFileName, const QString& videoAltText,
                                  int videoWidth, int videoHeight, ATProto::AppBskyFeed::Record::Post::SharedPtr post)
{
    emit postProgress(tr("Uploading video"));

    const QString fileName = videoFileName.sliced(7);
    auto file = std::make_shared<QFile>(fileName);
    if (!file->open(QFile::ReadOnly))
    {
        qWarning() << "Could not open video file:" << fileName;
        emit postFailed(tr("Could not open video file"));
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->uploadVideo(file.get(),
        [this, presence=getPresence(), videoAltText, videoWidth, videoHeight, post, file](ATProto::AppBskyVideo::JobStatus::SharedPtr output){
            if (!presence)
                return;

            if (!postMaster())
                return;

            postMaster()->addVideoToPost(post, *output, videoWidth, videoHeight, videoAltText,
                [this, presence, post]{
                    if (presence)
                       continuePost(post);
                },
                [this, presence](const QString& error, const QString& msg){
                    if (!presence)
                        return;

                    qDebug() << "Post failed:" << error << " - " << msg;
                    emit postFailed(msg);
                },
                [this, presence](const QString& status, std::optional<int> progress){
                    if (!presence)
                        return;

                    qDebug() << "Status:" << status << "progress:" << progress.value_or(-1);
                    QString msg(tr("Processing video: %1").arg(status));

                    if (progress)
                        msg += QString(" %1%").arg(*progress);

                    emit postProgress(msg);
                });
        },
        [this, presence=getPresence(), file](const QString& error, const QString& msg){
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

    // TODO: add feed interactions: reply, quote
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

            if (post->mEmbed)
            {
                if (post->mEmbed->mType == ATProto::AppBskyEmbed::EmbedType::RECORD)
                {
                    const auto& record = std::get<ATProto::AppBskyEmbed::Record::SharedPtr>(post->mEmbed->mEmbed);
                    Q_ASSERT(record);
                    Q_ASSERT(record->mRecord);

                    if (record && record->mRecord)
                    {
                        mSkywalker->makeLocalModelChange(
                            [record](LocalPostModelChanges* model){
                                model->updateQuoteCountDelta(record->mRecord->mCid, 1);
                            });
                    }
                }
                else if (post->mEmbed->mType == ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA)
                {
                    const auto& recordWithMedia = std::get<ATProto::AppBskyEmbed::RecordWithMedia::SharedPtr>(post->mEmbed->mEmbed);
                    Q_ASSERT(recordWithMedia);
                    Q_ASSERT(recordWithMedia->mRecord);
                    Q_ASSERT(recordWithMedia->mRecord->mRecord);

                    if (recordWithMedia && recordWithMedia->mRecord && recordWithMedia->mRecord->mRecord)
                    {
                        mSkywalker->makeLocalModelChange(
                            [recordWithMedia](LocalPostModelChanges* model){
                                model->updateQuoteCountDelta(recordWithMedia->mRecord->mRecord->mCid, 1);
                            });
                    }
                }
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

void PostUtils::repost(const QString& uri, const QString& cid,
                       const QString& viaUri, const QString& viaCid,
                       const QString& feedDid, const QString& feedContext)
{
    if (!postMaster())
        return;

    emit repostProgress(tr("Reposting"));

    postMaster()->checkRecordExists(uri, cid,
        [this, presence=getPresence(), uri, cid, viaUri, viaCid, feedDid, feedContext]{
            if (presence)
                continueRepost(uri, cid, viaUri, viaCid, feedDid, feedContext);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg;
            emit repostFailed(msg);
        });
}

void PostUtils::continueRepost(const QString& uri, const QString& cid,
                               const QString& viaUri, const QString& viaCid,
                               const QString& feedDid, const QString& feedContext)
{
    if (!postMaster())
        return;

    postMaster()->repost(uri, cid, viaUri, viaCid,
        [this, presence=getPresence(), uri, cid, feedDid, feedContext](const auto& repostUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, repostUri](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(cid, 1);
                    model->updateRepostUri(cid, repostUri);
                });

            mSkywalker->addFeedInteraction(
                feedDid, ATProto::AppBskyFeed::Interaction::EventType::InteractionRepost,
                uri, feedContext);

            emit repostOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg;
            emit repostFailed(msg);
        });
}

void PostUtils::undoRepost(const QString& repostUri, const QString& origPostUri,
                           const QString& origPostCid, const QString& feedDid)
{
    if (!postMaster())
        return;

    postMaster()->undo(repostUri,
        [this, presence=getPresence(), origPostUri, origPostCid, feedDid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [origPostCid](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(origPostCid, -1);
                    model->updateRepostUri(origPostCid, "");
                });

            mSkywalker->removeFeedInteraction(
                feedDid, ATProto::AppBskyFeed::Interaction::EventType::InteractionRepost,
                origPostUri);

            emit undoRepostOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo repost failed:" << error << " - " << msg;
            emit undoRepostFailed(msg);
        });
}

void PostUtils::like(const QString& uri, const QString& cid,
                     const QString& viaUri, const QString& viaCid,
                     const QString& feedDid, const QString& feedContext)
{
    if (!postMaster())
        return;

    mSkywalker->makeLocalModelChange(
        [cid](LocalPostModelChanges* model){
            model->updateLikeTransient(cid, true);
        });


    postMaster()->like(uri, cid, viaUri, viaCid,
        [this, presence=getPresence(), uri, cid, feedDid, feedContext](const auto& likeUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [uri, cid, feedDid, feedContext, likeUri](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, 1);
                    model->updateLikeUri(cid, likeUri);
                    model->updateLikeTransient(cid, false);
                });

            mSkywalker->addFeedInteraction(
                feedDid,
                ATProto::AppBskyFeed::Interaction::EventType::InteractionLike,
                uri, feedContext);

            emit likeOk();
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateLikeTransient(cid, false);
                });

            emit likeFailed(msg);
        });
}

void PostUtils::undoLike(const QString& likeUri, const QString& uri, const QString& cid, const QString& feedDid)
{
    if (!postMaster())
        return;

    mSkywalker->makeLocalModelChange(
        [cid](LocalPostModelChanges* model){
            model->updateLikeTransient(cid, true);
        });

    postMaster()->undo(likeUri,
        [this, presence=getPresence(), uri, cid, feedDid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [uri, cid, feedDid](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, -1);
                    model->updateLikeUri(cid, "");
                    model->updateLikeTransient(cid, false);
                });

            mSkywalker->removeFeedInteraction(
                feedDid,
                ATProto::AppBskyFeed::Interaction::EventType::InteractionLike,
                uri);

            emit undoLikeOk();
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg;

            mSkywalker->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateLikeTransient(cid, false);
                });

            emit undoLikeFailed(msg);
        });
}

void PostUtils::muteThread(const QString& uri)
{
    if (!bskyClient())
        return;

    bskyClient()->muteThread(uri,
        [this, presence=getPresence(), uri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [uri](LocalPostModelChanges* model){
                    model->updateThreadMuted(uri, true);
                });

            emit muteThreadOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Mute thread failed:" << error << " - " << msg;
            emit muteThreadFailed(msg);
        });
}

void PostUtils::unmuteThread(const QString& uri)
{
    if (!bskyClient())
        return;

    bskyClient()->unmuteThread(uri,
        [this, presence=getPresence(), uri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [uri](LocalPostModelChanges* model){
                    model->updateThreadMuted(uri, false);
                });

            emit unmuteThreadOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unute thread failed:" << error << " - " << msg;
            emit unmuteThreadFailed(msg);
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

        auto deleteRecord = std::make_shared<ATProto::ComATProtoRepo::ApplyWritesDelete>();
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

bool PostUtils::pickPhoto(bool pickVideo)
{
    const bool permission = PhotoPicker::pickPhoto(pickVideo);

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
    PhotoPicker::savePhoto(imageReader(), sourceUrl, false,
        [this, presence=getPresence()](const QString& fileName){
            if (!presence)
                return;

            qDebug() << "Saved photo:" << fileName;
            mSkywalker->showStatusMessage(tr("Picture saved"), QEnums::STATUS_LEVEL_INFO);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            mSkywalker->showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

void PostUtils::sharePhotoToApp(const QString& sourceUrl)
{
    qDebug() << "Share photo to app:" << sourceUrl;
#ifdef Q_OS_ANDROID
    PhotoPicker::savePhoto(imageReader(), sourceUrl, true,
        [this, presence=getPresence()](const QString& fileName){
            if (!presence)
                return;

            qDebug() << "Saved photo to cache:" << fileName;
            continueSharePhotoToApp(fileName);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            mSkywalker->showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
#else
    PhotoPicker::copyPhotoToClipboard(imageReader(), sourceUrl,
        [this, presence=getPresence()](){
            if (!presence)
                return;

            mSkywalker->showStatusMessage("Copied photo to clipboard", QEnums::STATUS_LEVEL_INFO);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            mSkywalker->showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
#endif
}

void PostUtils::continueSharePhotoToApp(const QString& fileName)
{
    qDebug() << "Share photo to app:" << fileName;
#ifdef Q_OS_ANDROID
    QJniObject jFileName = QJniObject::fromString(fileName);

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareMedia",
                                       "(Ljava/lang/String;)V",
                                       jFileName.object<jstring>());
#else
    mSkywalker->showStatusMessage("Sharing not supported", QEnums::STATUS_LEVEL_ERROR);
#endif
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

QString PostUtils::linkiFy(const QString& text, const QString& colorName)
{
    return ATProto::RichTextMaster::linkiFy(text, {}, colorName);
}

void PostUtils::getQuotePost(const QString& httpsUri)
{
    if (!postMaster())
        return;

    postMaster()->getPost(httpsUri,
        [this, presence=getPresence()](const auto& uri, const auto& cid, auto post, auto author){
            if (!presence)
                return;

            BasicProfile profile(author);
            const auto formattedText = ATProto::RichTextMaster::getFormattedPostText(*post, UserSettings::getCurrentLinkColor());
            emit quotePost(uri, cid, formattedText, profile, post->mCreatedAt);
        });
}

void PostUtils::getQuoteFeed(const QString& httpsUri)
{
    if (!postMaster())
        return;

    postMaster()->getFeed(httpsUri,
        [this, presence=getPresence()](auto feed){
            if (!presence)
                return;

            GeneratorView view(feed);
            emit quoteFeed(view);
        });
}

void PostUtils::getQuoteList(const QString& httpsUri)
{
    if (!postMaster())
        return;

    postMaster()->getList(httpsUri,
        [this, presence=getPresence()](auto list){
            if (!presence)
                return;

            ListView view(list);
            emit quoteList(view);
        });
}

void PostUtils::getPostgate(const QString& postUri)
{
    if (!postMaster())
        return;


    postMaster()->getPostgate(postUri,
        [this, presence=getPresence()](auto postgate){
            if (!presence)
                return;

            emit getPostgateOk(Postgate{postgate});
        },
        [this, presence=getPresence(), postUri](const QString& error, const QString& msg){
            if (!presence)
                return;

            if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
            {
                qDebug() << "No postgate record exists:" << postUri;
                emit getPostgateOk(Postgate{});
                return;
            }

            qWarning() << "Failed to get postgate:" << error << "-" << msg;
            emit getPostgateFailed(msg);
        });
}

void PostUtils::getVideoUploadLimits(const std::function<void(const VideoUploadLimits&)>& cb)
{
    Q_ASSERT(cb);
    qDebug() << "Get video upload limits";

    if (!bskyClient())
        return;

    bskyClient()->getVideoUploadLimits(
        [cb](auto output){
            const VideoUploadLimits limits(output);
            qDebug() << "Video upload limits:" << limits.canUpload() << "daily:" << limits.getRemainingDailyVideos() << limits.getRemainingDailyBytes() << limits.getError() << limits.getMessage();
            cb(limits);
        },
        [cb](const QString& error, const QString& msg){
            qWarning() << "Failed to get video upload limits:"  << error << "-" << msg;
            cb(VideoUploadLimits(error, msg));
        });
}

void PostUtils::getVideoUploadLimits()
{
    getVideoUploadLimits(
        [this, presence=getPresence()](const VideoUploadLimits& limits){
            if (!presence)
                return;

            emit videoUploadLimits(limits);
        });
}

void PostUtils::checkVideoUploadLimits()
{
    emit postProgress(tr("Checking limits"));

    getVideoUploadLimits(
        [this, presence=getPresence()](const VideoUploadLimits& limits){
            if (!presence)
                return;

            qDebug() << "Can upload:" << limits.canUpload() << limits.getError() << limits.getMessage();


            if (limits.canUpload())
                emit checkVideoLimitsOk(limits);
            else
                emit checkVideoLimitsFailed(!limits.getMessage().isEmpty() ? limits.getMessage() : limits.getError());
        });
}

PostInteractionSettings PostUtils::getPostInteractionSettings() const
{
    Q_ASSERT(mSkywalker);
    const auto& userPrefs = mSkywalker->userPreferences();
    return PostInteractionSettings(userPrefs.getPostInteractionSettingsPref());
}

void PostUtils::savePostInteractionSettings(bool allowMention, bool allowFollower, bool allowFollowing,
        const QStringList& allowList, bool allowNobody, bool disableEmbedding)
{
    qDebug() << "mention:" << allowMention << "follower:" << allowFollower << "following:" << allowFollowing << "list:" << allowList << "nobody:" << allowNobody << "disableEmbed:" << disableEmbedding;

    Q_ASSERT(mSkywalker);
    ATProto::AppBskyActor::PostInteractionSettingsPref pref;
    pref.mRules.mAllowMention = allowMention;
    pref.mRules.mAllowFollower = allowFollower;
    pref.mRules.mAllowFollowing = allowFollowing;
    pref.mRules.mAllowNobody = allowNobody;
    pref.mDisableEmbedding = disableEmbedding;

    for (const auto& list : allowList)
    {
        auto listRule = std::make_shared<ATProto::AppBskyFeed::ThreadgateListRule>();
        listRule->mList = list;
        pref.mRules.mAllowList.push_back(listRule);
    }

    auto userPrefs = mSkywalker->userPreferences();
    userPrefs.setPostInteractionSettingsPref(pref);
    mSkywalker->saveUserPreferences(userPrefs);
}

void PostUtils::identifyLanguage(QString text, int index)
{
    if (text.length() < MIN_LANGUAGE_IDENTIFICATION_LENGTH)
        return;

#if defined(Q_OS_ANDROID)
    auto jsText  = QJniObject::fromString(text);
    const int requestId = sNextRequestId++;
    addIndexLanguageIdentificationRequestId(index, requestId);
    const QStringList excludeLanguages = mSkywalker->getUserSettings()->getExcludeDetectLanguages(mSkywalker->getUserDid());
    auto jsExcludeLanguages = QJniObject::fromString(excludeLanguages.join(','));

    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/LanguageDetection",
        "detectLanguage",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        jsText,
        jsExcludeLanguages,
        (jint)requestId);
#else
    qDebug() << "Language identification not supported:" << index << text;
#endif
}

void PostUtils::shareMedia(int fd, const QString& mimeType)
{
    if (!mPickingPhoto)
        return;

    mPickingPhoto = false;

    if (fd < 0)
    {
        qWarning() << "No file descriptor:" << fd;
        emit photoPickFailed(tr("Failed to open media"));
        return;
    }

    if (mimeType.startsWith("image"))
    {
        sharePhoto(fd);
    }
    else if (mimeType.startsWith("video"))
    {
        shareVideo(fd, mimeType);
    }
    else
    {
        qWarning() << "Unsupported mime type:" << mimeType;
        emit photoPickFailed(QString("Unsupported media type: %1").arg(mimeType));
    }
}

void PostUtils::sharePhoto(int fd)
{
    qDebug() << "Share photo fd:" << fd;
    auto [img, gifTempFileName, error] = PhotoPicker::readImageFd(fd);

    if (img.isNull())
    {
        emit photoPickFailed(error);
        return;
    }

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString source = imgProvider->addImage(img);
    emit photoPicked(source, gifTempFileName);
}

void PostUtils::shareVideo(int fd, const QString& mimeTypeName)
{
    qDebug() << "Share video fd:" << fd << mimeTypeName;
    QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForName(mimeTypeName);
    QString ext = mimeType.preferredSuffix();

    if (ext.isEmpty() && mimeTypeName == "video/mp2ts")
        ext = "ts";

    if (ext.isEmpty())
    {
        emit videoPickedFailed(tr("Unsupported mime type: %1").arg(mimeTypeName));
        return;
    }

    auto video = FileUtils::createTempFile(fd, ext);

    if (!video)
    {
        emit videoPickedFailed(tr("Cannot create video file"));
        return;
    }

    const QString tmpFilePath = video->fileName();
    TempFileHolder::instance().put(std::move(video));
    QUrl url = QUrl::fromLocalFile(tmpFilePath);
    qDebug() << "Video url:" << url;
    emit videoPicked(url);
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

void PostUtils::dropVideo(const QString& source)
{
    qDebug() << "Drop video:" << source;

    if (source.startsWith("file://"))
    {
        const QString fileName = source.sliced(7);
        TempFileHolder::instance().remove(fileName);
    }
}

void PostUtils::handleLanguageIdentified(const QString& languageCode, int requestId)
{
    if (!mLanguageIdentificationRequestIdIndexMap.contains(requestId))
        return;

    const int index = mLanguageIdentificationRequestIdIndexMap[requestId];
    removeIndexLanguageIdentificationRequestId(index, requestId);

    if (!languageCode.isEmpty() && languageCode != "und")
    {
        if (LanguageUtils::existsShortCode(languageCode))
            emit languageIdentified(languageCode, index);
        else
            qWarning() << "Unknown language code:" << languageCode;
    }
}

void PostUtils::addIndexLanguageIdentificationRequestId(int index, int requestId)
{
    mIndexLanguageIdentificationRequestIdMap[index] = requestId;
    mLanguageIdentificationRequestIdIndexMap[requestId] = index;
}

void PostUtils::removeIndexLanguageIdentificationRequestId(int index, int requestId)
{
    mIndexLanguageIdentificationRequestIdMap.erase(index);
    mLanguageIdentificationRequestIdIndexMap.erase(requestId);
}

}
