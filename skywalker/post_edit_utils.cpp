// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "post_edit_utils.h"
#include "abstract_post_feed_model.h"
#include "draft_post_data.h"
#include "draft_posts.h"
#include "file_utils.h"
#include "gif_utils.h"
#include "shared_image_provider.h"
#include "skywalker.h"
#include "temp_file_holder.h"
#include "thread_unroller.h"
#include <QRegularExpression>

namespace Skywalker {

PostEditUtils::PostEditUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
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

static void removePostThreadCounter(DraftPostData* data)
{
    static const QRegularExpression countRE{ R"(.*([0-9]+) */.*)" };

    const QString counter = ThreadUnroller::getCounter(data->text());

    if (counter.isEmpty())
        return;

    QString text = data->text();
    ThreadUnroller::removeCounterFromPlainText(text, counter);
    data->setText(text);

    auto match = countRE.match(counter);

    if (!match.hasMatch())
        return;

    qDebug() << "Counter match:" << match.captured(0) << "number:" << match.captured(1);
    data->setPostThreadCount(match.captured(1).toInt());
}

// For draft post more GIF properties are saved, that we do no have
// amymore for a real post. Overwrite the draft post GIF with one created
// from a real post.
static void setGif(DraftPostData* data, const Post& post)
{
    data->setGif({});
    const auto externalView = post.getExternalView();

    if (!externalView)
        return;

    GifUtils gifUtils;

    if (!gifUtils.isTenorLink(externalView->getUri()))
        return;

    const QUrl uri(externalView->getUri());
    const QUrlQuery query(uri.query());

    // The ww and hh parameters are added to the link when posting.
    if (!query.hasQueryItem("ww") || !query.hasQueryItem("hh"))
    {
        qWarning() << "Missing size for editing GIF:" << uri;
        return;
    }

    const QString gifUrl = uri.toString(QUrl::RemoveQuery);
    const int gifWidth = query.queryItemValue("ww").toInt();
    const int gifHeight = query.queryItemValue("hh").toInt();
    const QSize gifSize(gifWidth, gifHeight);

    // NOTE: The id is set to empty. This will skip registration in Tenor::registerShare
    TenorGif gif("", externalView->getTitle(), "",
                 gifUrl, gifSize,
                 gifUrl, gifSize,
                 externalView->getThumbUrl(), QSize(1, 1));

    data->setGif(gif);
}

void PostEditUtils::getEditPostData(AbstractPostFeedModel* model, int postIndex)
{
    clearState();

    if (!model)
    {
        qWarning() << "Model not set";
        emit editPostDataFailed("Internal failure");
        return;
    }

    const Post& post = model->getPost(postIndex);

    if (post.isPlaceHolder())
    {
        qWarning() << "Post place holder, index:" << postIndex;
        emit editPostDataFailed(tr("Failed to retrieve post"));
        return;
    }

    emit editPostDataProgress(tr("Fetching post for editing"));

    getPostThread(post.getUri(),
        [this, presence=getPresence(), post](int modelId){
            if (!presence)
                return;

            if (checkPostStats(modelId))
                getEditPostData(modelId, post);
            else
                saveState(modelId, post);
        },
        [this, presence=getPresence()](QString error){
            if (presence)
                emit editPostDataFailed(error);
        });
}

void PostEditUtils::resume()
{
    if (mPostThreadModelId < 0)
    {
        qWarning() << "No model to resume:" << mPostThreadModelId;
        emit editPostDataFailed("Internal failure");
        return;
    }

    getEditPostData(mPostThreadModelId, mEntryPost);
}

void PostEditUtils::cancel()
{
    if (mPostThreadModelId >= 0)
        mSkywalker->removePostThreadModel(mPostThreadModelId);

    clearState();
}

void PostEditUtils::clearState()
{
    mPostThreadModelId = -1;
    mEntryPost = Post{};
    mPostMaster = nullptr;
    mImageReader = nullptr;
    mM3U8Reader = nullptr;
}

void PostEditUtils::saveState(int postThreadModelId, const Post& post)
{
    mPostThreadModelId = postThreadModelId;
    mEntryPost = post;
}

bool PostEditUtils::checkPostStats(int postThreadModelId)
{
    auto* model = mSkywalker->getPostThreadModel(postThreadModelId);

    if (!model)
    {
        qWarning() << "Model not found:" << postThreadModelId;

        // The post edit procedure will property fail later.
        return true;
    }

    int reply = 0;
    int repost = 0;
    int quote = 0;
    int like = 0;

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const Post& post = model->getPost(i);
        reply += post.getReplyCount();
        repost += post.getRepostCount();
        quote += post.getQuoteCount();
        like += post.getLikeCount();
    }

    if (model->rowCount() > 1)
    {
        // Do not count own thread replies
        reply -= model->rowCount() - 1;

        if (reply < 0)
        {
            qWarning() << "Replies got negative:" << reply << "threadCount:" << model->rowCount();
            reply = 0;
        }
    }

    qDebug() << "Stats reply:" << reply << "repost:" << repost << "quote:" << quote << "like:" << like;

    if (reply + repost + quote + like == 0)
        return true;

    emit editPostPaused(reply, repost, quote, like);
    return false;
}

// NOTE: the entry post from the post feed model has a parent post when it is a reply.
// We want reply information in the post composer. The first post in the post thread model
// does not have this reply informaiton.
void PostEditUtils::getEditPostData(int postThreadModelId, const Post& entryPost)
{
    qDebug() << "Get post edit data, entry post:" << entryPost.getUri();
    getEditPostDataContinue(postThreadModelId, entryPost, {});
}

void PostEditUtils::getEditPostData(int postThreadModelId, const QList<DraftPostData*>& postData)
{
    qDebug() << "Get post edit data, posts already retrieved:" << postData.size();
    auto* model = mSkywalker->getPostThreadModel(postThreadModelId);

    if (!model)
    {
        qWarning() << "Model not found:" << postThreadModelId;
        emit editPostDataFailed("Internal failure");
        return;
    }

    if (postData.size() >= model->rowCount())
    {
        qDebug() << "Got all posts for editing, modelId:" << postThreadModelId;
        mSkywalker->removePostThreadModel(postThreadModelId);
        emit editPostDataOk(postData);
        return;
    }

    const Post& post = model->getPost(postData.size());
    getEditPostDataContinue(postThreadModelId, post, postData);
}

void PostEditUtils::getEditPostDataContinue(int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData)
{
    if (post.isPlaceHolder())
    {
        qWarning() << "Post place holder, modelId:" << postThreadModelId << "index:" << postData.size();
        mSkywalker->removePostThreadModel(postThreadModelId);
        emit editPostDataFailed(tr("Failed to retrieve post"));
        return;
    }

    auto* data = new DraftPostData(this);
    data->setUri(post.getUri());
    data->setCid(post.getCid());
    DraftPosts::setDraftPost(data, post);
    data->setEmbeddedLinks(post.getEmbeddedLinks());
    setGif(data, post);
    removePostThreadCounter(data);

    if (postData.empty())
    {
        auto* model = mSkywalker->getPostThreadModel(postThreadModelId);

        if (model)
        {
            // Threadgate settings are on the first post in the post thread model,
            // not on the entry post
            const Post& firstPost = model->getPost(postData.size());
            DraftPosts::setReplyRestrictions(data, firstPost);
        }

        loadEditPostGate(data, postThreadModelId, post, postData);
    }
    else
    {
        loadEditPostImages(data, postThreadModelId, post, postData);
    }
}

void PostEditUtils::loadEditPostGate(DraftPostData* data, int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData)
{
    if (!postMaster())
    {
        mSkywalker->removePostThreadModel(postThreadModelId);
        emit editPostDataFailed(tr("Internal failure"));
        return;
    }

    emit editPostDataProgress(tr("Loading post restrictions"));

    postMaster()->getPostgate(post.getUri(),
        [this, presence=getPresence(), data, postThreadModelId, post, postData](auto postgate){
            if (!presence)
                return;

            data->setEmbeddingDisabled(postgate->mDisableEmbedding);
            loadEditPostImages(data, postThreadModelId, post, postData);
        },
        [this, presence=getPresence(), data, postThreadModelId, post, postData](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Failed to get postgate:" << error << "-" << msg;
            data->setEmbeddingDisabled(false);
            loadEditPostImages(data, postThreadModelId, post, postData);
        });
}

void PostEditUtils::loadEditPostImages(DraftPostData* data, int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData, int imageIndex)
{
    auto recordWithMediaVew = post.getRecordWithMediaView();
    const QList<ImageView> images = recordWithMediaVew ? recordWithMediaVew->getImages() : post.getImages();

    if (imageIndex < 0 || imageIndex >= images.size())
    {
        loadEditPostVideo(data, postThreadModelId, post, postData);
        return;
    }

    emit editPostDataProgress(tr("Loading post #%1 image #%2").arg(postData.size() + 1).arg(imageIndex + 1));

    imageReader()->getImage(images[imageIndex].getFullSizeUrl(),
        [this, presence=getPresence(), images, data, postThreadModelId, post, postData, imageIndex](QImage img){
            if (!presence)
                return;

            if (!img.isNull())
            {
                auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
                const QString imgSource = imgProvider->addImage(img);
                auto draftImages = data->images();
                const ImageView draftImaage(imgSource, images[imageIndex].getAlt());
                draftImages.push_back(draftImaage);
                data->setImages(draftImages);
            }

            loadEditPostImages(data, postThreadModelId, post, postData, imageIndex + 1);
        },
        [this, presence=getPresence(), data, postThreadModelId, post, postData, imageIndex](const QString& error){
            if (!presence)
                return;

            qWarning() << "Failed get get image:" << error;
            loadEditPostImages(data, postThreadModelId, post, postData, imageIndex + 1);
        });
}

void PostEditUtils::loadEditPostVideo(DraftPostData* data, int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData)
{
    auto recordWithMediaVew = post.getRecordWithMediaView();
    VideoView::Ptr videoView = recordWithMediaVew ? recordWithMediaVew->getVideoView() : post.getVideoView();

    if (!videoView)
    {
        finishedLoadingEditPost(data, postThreadModelId, postData);
        return;
    }

    const QString url = videoView->getPlaylistUrl();
    qDebug() << "Load video:" << url << "width:" << videoView->getWidth() << "height:" << videoView->getHeight();

    if (!url.endsWith(".m3u8"))
    {
        qWarning() << "Cannot load video:" << url;
        finishedLoadingEditPost(data, postThreadModelId, postData);
        return;
    }

    emit editPostDataProgress(tr("Loading post video"));

    auto* videoReader = m3u8Reader();

    connect(videoReader, &M3U8Reader::getVideoStreamOk, this,
            [this](int){
                m3u8Reader()->loadStream();
            });
    connect(videoReader, &M3U8Reader::getVideoStreamError, this,
            [this, data, postThreadModelId, postData]{
                finishedLoadingEditPost(data, postThreadModelId, postData);
            });
    connect(videoReader, &M3U8Reader::loadStreamOk, this,
            [this, data, postThreadModelId, post, postData](QString videoStream){
                loadEditPostVideo(data, postThreadModelId, post, videoStream, postData);
            });
    connect(videoReader, &M3U8Reader::loadStreamError, this,
            [this, data, postThreadModelId, postData]{
                finishedLoadingEditPost(data, postThreadModelId, postData);
            });

    videoReader->getVideoStream(url);
}

void PostEditUtils::loadEditPostVideo(DraftPostData* data, int postThreadModelId, const Post& post, const QString& videoStream, const QList<DraftPostData*>& postData)
{
    qDebug() << "Load post video:" << videoStream;
    auto recordWithMediaVew = post.getRecordWithMediaView();
    VideoView::Ptr videoView = recordWithMediaVew ? recordWithMediaVew->getVideoView() : post.getVideoView();
    Q_ASSERT(videoView);

    auto tmpFile = FileUtils::createTempFile(videoStream, "ts");
    const QUrl url = QUrl::fromLocalFile(tmpFile->fileName());
    VideoView draftVideo(url.toString(), videoView->getAlt(), 0, 0, false, videoView->getHeight());
    data->setVideo(draftVideo);
    TempFileHolder::instance().put(std::move(tmpFile));

    finishedLoadingEditPost(data, postThreadModelId, postData);
}

void PostEditUtils::finishedLoadingEditPost(DraftPostData* data, int postThreadModelId, const QList<DraftPostData*>& postData)
{
    qDebug() << "Add post for editing, modelId:" << postThreadModelId;
    clearState();
    QList<DraftPostData*> draftPostData{postData};
    draftPostData.push_back(data);
    getEditPostData(postThreadModelId, draftPostData);
}

void PostEditUtils::getPostThread(const QString& uri,
                   const std::function<void(int modelId)>& successCb,
                   const std::function<void(QString error)>& errorCb)
{
    qDebug() << "Get post thread:" << uri;

    if (!bskyClient())
    {
        emit errorCb("Internal failure");
        return;
    }

    if (mInProgress)
    {
        qDebug() << "In progress";
        emit errorCb(tr("Busy, try later"));
        return;
    }

    setInProgress(true);
    bskyClient()->getPostThread(uri, {}, 0,
        [this, presence=getPresence(), uri, successCb, errorCb](auto thread){
            if (!presence)
                return;

            setInProgress(false);
            const int modelId = mSkywalker->createPostThreadModel(uri, QEnums::POST_THREAD_ENTRY_AUTHOR_POSTS);
            auto model = mSkywalker->getPostThreadModel(modelId);
            Q_ASSERT(model);

            int postEntryIndex = model->setPostThread(thread);

            if (postEntryIndex < 0)
            {
                qDebug() << "No thread posts";
                mSkywalker->removePostThreadModel(modelId);
                errorCb(tr("Failed to retrieve post"));
                return;
            }

            const QString attachUri = model->getPostToAttachMore();

            if (!attachUri.isEmpty())
                addPostThread(attachUri, modelId, successCb, errorCb);
            else
                successCb(modelId);
        },
        [this, presence=getPresence(), errorCb](const QString& error, const QString& msg){
            if (!presence)
                return;

            setInProgress(false);
            qDebug() << "getPostThread FAILED:" << error << " - " << msg;
            errorCb(msg);
        });
}

void PostEditUtils::addPostThread(const QString& uri, int modelId,
                   const std::function<void(int modelId)>& successCb,
                   const std::function<void(QString error)>& errorCb,
                   int maxPages)
{
    Q_ASSERT(modelId >= 0);
    qDebug() << "Add post thread:" << uri << "model:" << modelId << "maxPages:" << maxPages;

    if (!bskyClient())
    {
        emit errorCb("Internal failure");
        return;
    }

    if (mInProgress)
    {
        qDebug() << "In progress";
        mSkywalker->removePostThreadModel(modelId);
        emit errorCb(tr("Busy, try later"));
        return;
    }

    auto model = mSkywalker->getPostThreadModel(modelId);

    if (!model)
    {
        qWarning() << "No model:" << modelId;
        emit errorCb("Internal failure");
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        mSkywalker->removePostThreadModel(modelId);
        errorCb(tr("Post thread is too long to edit"));
        return;
    }

    setInProgress(true);
    bskyClient()->getPostThread(uri, {}, 0,
        [this, presence=getPresence(), modelId, maxPages, successCb, errorCb](auto thread){
            if (!presence)
                return;

            setInProgress(false);
            auto model = mSkywalker->getPostThreadModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                errorCb("Internal failure");
                return;
            }

            if (model->addMorePosts(thread))
            {
                const QString leafUri = model->getPostToAttachMore();

                if (!leafUri.isEmpty())
                {
                    addPostThread(leafUri, modelId, successCb, errorCb, maxPages - 1);
                    return;
                }
            }

            qDebug() << "No more posts to add";
            successCb(modelId);
        },
        [this, presence=getPresence(), modelId, errorCb](const QString& error, const QString& msg){
            if (!presence)
                return;

            setInProgress(false);
            qDebug() << "getPostThread FAILED:" << error << " - " << msg;
            mSkywalker->removePostThreadModel(modelId);
            errorCb(msg);
    });
}

void PostEditUtils::setInProgress(bool inProgress)
{
    if (inProgress != mInProgress)
    {
        mInProgress = inProgress;
        emit inProgressChanged();
    }
}

ATProto::PostMaster* PostEditUtils::postMaster()
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

ImageReader* PostEditUtils::imageReader()
{
    if (!mImageReader)
        mImageReader = std::make_unique<ImageReader>(this);

    return mImageReader.get();
}

M3U8Reader* PostEditUtils::m3u8Reader()
{
    if (!mM3U8Reader)
        mM3U8Reader = std::make_unique<M3U8Reader>(this);

    return mM3U8Reader.get();
}

}
