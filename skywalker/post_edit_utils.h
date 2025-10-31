// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include "m3u8_reader.h"
#include "post.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class AbstractPostFeedModel;
class DraftPostData;

class PostEditUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PostEditUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getEditPostData(AbstractPostFeedModel* model, int postIndex);
    Q_INVOKABLE void resume();
    Q_INVOKABLE void cancel();

signals:
    void editPostPaused(int reply, int repost, int quote, int like);
    void editPostDataOk(QList<DraftPostData*> draft);
    void editPostDataProgress(QString msg);
    void editPostDataFailed(QString error);
    void inProgressChanged();

private:
    void clearState();
    void saveState(int postThreadModelId, const Post& post);
    bool checkPostStats(int postThreadModelId);
    void getEditPostData(int postThreadModelId, const Post& entryPost);
    void getEditPostData(int postThreadModelId, const QList<DraftPostData*>& postData);
    void getEditPostDataContinue(int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData);
    void loadEditPostImages(DraftPostData* data, int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData, int imageIndex = 0);
    void loadEditPostVideo(DraftPostData* data, int postThreadModelId, const Post& post, const QList<DraftPostData*>& postData);
    void loadEditPostVideo(DraftPostData* data, int postThreadModelId, const Post& post, const QString& videoStream, const QList<DraftPostData*>& postData);
    void finishedLoadingEditPost(DraftPostData* data, int postThreadModelId, const QList<DraftPostData*>& postData);

    void getPostThread(const QString& uri,
                       const std::function<void(int modelId)>& successCb,
                       const std::function<void(QString error)>& errorCb);

    void addPostThread(const QString& uri, int modelId,
                       const std::function<void(int modelId)>& successCb,
                       const std::function<void(QString error)>& errorCb,
                       int maxPages = 20);

    void setInProgress(bool inProgress);

    ImageReader* imageReader();
    M3U8Reader* m3u8Reader();

    std::unique_ptr<ImageReader> mImageReader;
    std::unique_ptr<M3U8Reader> mM3U8Reader;
    bool mInProgress = false;
    int mPostThreadModelId = -1;
    Post mEntryPost;
};

}
