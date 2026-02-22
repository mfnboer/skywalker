// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "draft_posts_model.h"
#include "atproto_image_provider.h"
#include "content_filter.h"
#include "file_utils.h"
#include "gif_utils.h"
#include "link_card_reader.h"
#include "photo_picker.h"
#include "skywalker.h"
#include "temp_file_holder.h"
#include "utils.h"
#include "lexicon/lexicon.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* DRAFT_POSTS_DIR = "sw-draft-posts";
constexpr char const* DRAFT_PICTURES_DIR = "SkywalkerDrafts";

QString createAbsPath(const QString& draftsPath, const QString& fileName)
{
    return QString("%1/%2").arg(draftsPath, fileName);
}

}

DraftPosts::DraftPosts(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
}

DraftPosts::~DraftPosts()
{
    auto* imgProvider = ATProtoImageProvider::getProvider(ATProtoImageProvider::DRAFT_IMAGE);
    imgProvider->clear();
}

void DraftPosts::setStorageType(StorageType storageType)
{
    if (storageType != mStorageType)
    {
        mStorageType = storageType;
        emit storageTypeChanged();
    }
}

bool DraftPosts::hasDrafts() const
{
    return mDraftPostsModel && mDraftPostsModel->rowCount() > 0;
}

bool DraftPosts::canSaveDraft() const
{
    switch (mStorageType)
    {
    case STORAGE_FILE:
        return mDraftPostsModel->rowCount() < MAX_DRAFTS;
    case STORAGE_BLUESKY:
        return true;
    }

    return false;
}

DraftPostData* DraftPosts::createDraft(
    const QString& text,
    const WebLink::List& embeddedLinks,
    const QStringList& imageFileNames, const QStringList& altTexts,
    const QStringList& memeTopTexts, const QStringList& memeBottomTexts,
    const QString& videoFileName, bool videoIsGif, const QString& videoAltText,
    int videoStartMs, int videoEndMs, int videoNewHeight,
    bool videoRemoveAudio,
    const QString& replyToUri, const QString& replyToCid,
    const QString& replyRootUri, const QString& replyRootCid,
    const BasicProfile& replyToAuthor, const QString& replyToText,
    const QDateTime& replyToDateTime,
    const QString& quoteUri, const QString& quoteCid,
    const BasicProfile& quoteAuthor, const QString& quoteText,
    const QDateTime& quoteDateTime, bool quoteFixed,
    const GeneratorView& quoteFeed, const ListView& quoteList,
    const TenorGif gif, const LinkCard* card, const QStringList& labels,
    const QString& language,
    bool restrictReplies, bool allowMention, bool allowFollower, bool allowFollowing,
    const QStringList& allowLists, bool embeddingDisabled,
    QDateTime timestamp)
{
    auto* draft = new DraftPostData(this);
    draft->setText(text);
    draft->setEmbeddedLinks(embeddedLinks);

    QList<ImageView> images;
    Q_ASSERT(imageFileNames.size() == altTexts.size());
    Q_ASSERT(imageFileNames.size() == memeTopTexts.size());
    Q_ASSERT(imageFileNames.size() == memeBottomTexts.size());

    for (int i = 0; i < imageFileNames.size(); ++i)
    {
        ImageView view(imageFileNames[i], altTexts[i], memeTopTexts[i], memeBottomTexts[i]);
        images.push_back(view);
    }

    draft->setImages(images);

    if (!videoFileName.isEmpty())
    {
        const VideoView video(videoFileName, videoIsGif, videoAltText, videoStartMs, videoEndMs, videoRemoveAudio, videoNewHeight);
        draft->setVideo(video);
    }

    draft->setIndexedAt(timestamp);
    draft->setReplyToUri(replyToUri);
    draft->setReplyToCid(replyToCid);
    draft->setReplyRootUri(replyRootUri);
    draft->setReplyRootCid(replyRootCid);
    draft->setReplyToAuthor(replyToAuthor);
    draft->setReplyToText(replyToText);
    draft->setReplyToDateTime(replyToDateTime);
    draft->setQuoteUri(quoteUri);
    draft->setQuoteCid(quoteCid);
    draft->setQuoteAuthor(quoteAuthor);
    draft->setQuoteText(quoteText);
    draft->setQuoteDateTime(quoteDateTime);
    draft->setQuoteFixed(quoteFixed);
    draft->setQuoteFeed(quoteFeed);
    draft->setQuoteList(quoteList);
    draft->setGif(gif);

    if (card)
        draft->setExternalLink(card->getLink());

    draft->setLabels(labels);
    draft->setLanguage(language);
    draft->setRestrictReplies(restrictReplies);
    draft->setAllowMention(allowMention);
    draft->setAllowFollower(allowFollower);
    draft->setAllowFollowing(allowFollowing);
    draft->setAllowLists(allowLists);
    draft->setEmbeddingDisabled(embeddingDisabled);

    return draft;
}

static QString createPicBaseName(const QString& baseName, int postIndex)
{
    return QString("%1-%2").arg(baseName).arg(postIndex);
}

bool DraftPosts::saveDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread)
{
    qDebug() << "Save draft post:" << draftPost->text();

    switch (mStorageType)
    {
    case STORAGE_FILE:
        return saveFileDraftPost(draftPost, draftThread);
    case STORAGE_BLUESKY:
        return saveBlueskyDraftPost(draftPost, draftThread);
    }

    Q_ASSERT(false);
    return false;
}

bool DraftPosts::saveFileDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
    {
        emit saveDraftPostFailed(tr("Cannot create app data path"));
        return false;
    }

    const QString dateTime = FileUtils::createDateTimeName(draftPost->indexedAt());
    auto draft = std::make_shared<Draft::Draft>();
    draft->mPost = createPost(draftPost, createPicBaseName(dateTime, 0));

    if (!draft->mPost)
        return false;

    if (draftPost->restrictReplies())
    {
        const QString fileName = createDraftPostFileName(dateTime);
        const bool allowNobody = !draftPost->allowMention() && !draftPost->allowFollower() && !draftPost->allowFollowing() && draftPost->allowLists().empty();
        draft->mThreadgate = ATProto::PostMaster::createThreadgate(
            getDraftUri(fileName), draftPost->allowMention(), draftPost->allowFollower(), draftPost->allowFollowing(),
            draftPost->allowLists(), allowNobody, {});
    }

    draft->mReplyToPost = createReplyToPost(draftPost->replyToUri(), draftPost->replyToAuthor(),
                                            draftPost->replyToText(), draftPost->replyToDateTime());
    draft->mQuote = createQuote(draftPost->quoteUri(), draftPost->quoteAuthor(),
                                draftPost->quoteText(), draftPost->quoteDateTime(),
                                draftPost->quoteFeed(), draftPost->quoteList());

    draft->mEmbeddingDisabled = draftPost->embeddingDisabled();

    for (int i = 0; i < draftThread.size(); ++i)
    {
        const auto* draftThreadPost = draftThread[i];
        auto threadPost = createPost(draftThreadPost, createPicBaseName(dateTime, i + 1));

        if (!threadPost)
            return false;

        auto quote = createQuote(draftThreadPost->quoteUri(), draftThreadPost->quoteAuthor(),
                                 draftThreadPost->quoteText(), draftThreadPost->quoteDateTime(),
                                 draftThreadPost->quoteFeed(), draftThreadPost->quoteList());

        if (quote)
        {
            auto quoteJson = quote->toJson();
            threadPost->mJson.insert(Lexicon::DRAFT_POST_QUOTE_FIELD, quoteJson);
        }

        draft->mThreadPosts.push_back(std::move(threadPost));
    }

    if (!save(*draft, draftsPath, dateTime))
    {
        dropDraftMedia(draftPost, draftThread, dateTime);
        return false;
    }

    return true;
}

ATProto::AppBskyFeed::Record::Post::SharedPtr DraftPosts::createPost(const DraftPostData* draftPost, const QString& picBaseName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    QString picDraftsPath;

    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
    {
        emit saveDraftPostFailed(tr("Cannot create app data path"));
        return nullptr;
    }

    if (!draftPost->images().isEmpty() || !draftPost->video().isNull())
    {
        picDraftsPath = getPictureDraftsPath();

        if (picDraftsPath.isEmpty())
        {
            emit saveDraftPostFailed(tr("Cannot create picture drafts path"));
            return nullptr;
        }
    }

    ATProto::AppBskyFeed::PostReplyRef::SharedPtr replyRef = draftPost->replyToUri().isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(draftPost->replyToUri(), draftPost->replyToCid(),
                                                draftPost->replyRootUri(), draftPost->replyRootCid());

    auto post = ATProto::PostMaster::createPostWithoutFacets(draftPost->text(),
            draftPost->language(), std::move(replyRef));

    post->mCreatedAt = draftPost->indexedAt();
    ATProto::PostMaster::addLabelsToPost(*post, draftPost->labels());

    if (!draftPost->quoteUri().isEmpty())
        ATProto::PostMaster::addQuoteToPost(*post, draftPost->quoteUri(), draftPost->quoteCid());

    if (!addImagesToPost(*post, draftPost->images(), picDraftsPath, picBaseName))
        return nullptr;

    if (!addVideoToPost(*post, draftPost->video(), picDraftsPath, picBaseName))
        return nullptr;

    if (!draftPost->gif().isNull())
        addGifToPost(*post, draftPost->gif());
    else if (!draftPost->externalLink().isEmpty())
        addExternalLinkToPost(*post, draftPost->externalLink());

    if (!draftPost->embeddedLinks().empty())
    {
        auto links = createEmbeddedLinks(draftPost->embeddedLinks());
        post->mJson.insert(Lexicon::DRAFT_EMBBEDED_LINKS_FIELD, links->toJson());
    }

    return post;
}

void DraftPosts::loadDraftPosts()
{
    switch(mStorageType)
    {
    case STORAGE_FILE:
        loadDraftFeed();
        break;
    case STORAGE_BLUESKY:
        loadBlueskyDrafts();
        break;
    }
}

void DraftPosts::loadDraftPostsNextPage()
{
    if (mStorageType == STORAGE_BLUESKY)
        loadBlueskyDraftsNextPage();
}

DraftPostsModel* DraftPosts::getDraftPostsModel()
{
    if (!mDraftPostsModel)
    {
        mDraftPostsModel = mSkywalker->createDraftPostsModel();
        mDraftPostsModel->setDraftPosts(this);
    }

    return mDraftPostsModel.get();
}

static void setRecordViewData(DraftPostData* data, const RecordView* recordView)
{
    if (recordView->getAvailable())
    {
        const QString uri = recordView->getUri();
        const ATProto::ATUri atUri(uri);
        const QString httpsUri = atUri.toHttpsUri();

        if (!httpsUri.isEmpty())
            data->setOpenAsQuotePost(!data->text().contains(httpsUri));

        data->setQuoteUri(uri);
        data->setQuoteCid(recordView->getCid());
        data->setQuoteAuthor(recordView->getAuthor());
        data->setQuoteText(recordView->getText());
        data->setQuoteDateTime(recordView->getIndexedAt());
        data->setQuoteFixed(data->openAsQuotePost());
    }
    else if (recordView->getFeedAvailable())
    {
        const auto feed = recordView->getFeed();
        const QString uri = feed.getUri();
        const ATProto::ATUri atUri(uri);
        const QString httpsUri = atUri.toHttpsUri();

        data->setQuoteFeed(feed);
        data->setQuoteFixed(!httpsUri.isEmpty() && !feed.getDescription().contains(httpsUri));
    }
    else  if (recordView->getListAvailable())
    {
        const auto list = recordView->getList();
        const QString uri = list.getUri();
        const ATProto::ATUri atUri(uri);
        const QString httpsUri = atUri.toHttpsUri();

        data->setQuoteList(list);
        data->setQuoteFixed(!httpsUri.isEmpty() && !list.getDescription().contains(httpsUri));
    }
}

static void setImages(DraftPostData* data, const QList<ImageView>& images)
{
    QList<ImageView> draftImages;
    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);

    // Add images to image provider so the actual draft files can be removed
    for (const auto& imgView : images)
    {
        const QImage img = PhotoPicker::loadImage(imgView.getFullSizeUrl());

        if (!img.isNull())
        {
            const QString imgSource = imgProvider->addImage(img);
            ImageView draftImg(imgSource, imgView.getAlt(), imgView.getMemeTopText(), imgView.getMemeBottomText());
            draftImages.push_back(draftImg);
        }
    }

    data->setImages(draftImages);
}

static void setVideo(DraftPostData* data, const VideoView& videoView)
{
    if (videoView.isNull())
        return;

    auto tmpFile = FileUtils::createTempFile(videoView.getPlaylistUrl(), "mp4");

    if (!tmpFile)
        return;

    const QUrl url = QUrl::fromLocalFile(tmpFile->fileName());
    const bool isGif = videoView.getPresentation() == QEnums::VIDEO_PRESENTATION_GIF;
    VideoView video(url.toString(), isGif, videoView.getAlt(), videoView.getStartMs(), videoView.getEndMs(),
                    videoView.getRemoveAudio(), videoView.getNewHeight());
    data->setVideo(video);
    TempFileHolder::instance().put(std::move(tmpFile));
}

static void setExternal(DraftPostData* data, const ExternalView* externalView)
{
    GifUtils gifUtils;
    const QString link = externalView->getUri();

    if (gifUtils.isTenorLink(link) || gifUtils.isGiphyLink(link))
    {
        // NOTE: in addGifToPost several gif properties are packed into the URI as query params
        const QUrl uri(link);
        const QUrlQuery query(uri.query());
        const QString smallUrl = query.queryItemValue("smallUrl");
        const int smallWidth = query.queryItemValue("smallWidth").toInt();
        const int smallHeight = query.queryItemValue("smallHeight").toInt();

        int gifWidth = -1;
        int gifHeight= -1;

        if (query.hasQueryItem("gifWidth") && query.hasQueryItem("gifHeight"))
        {
            gifWidth = query.queryItemValue("gifWidth").toInt();
            gifHeight = query.queryItemValue("gifHeight").toInt();
        }

        // NOTE: The id is set to empty. This will avoid registration in Tenor::registerShare
        TenorGif gif("", externalView->getTitle(), "",
                     uri.toString(QUrl::RemoveQuery), QSize(gifWidth, gifHeight),
                     smallUrl, QSize(smallWidth, smallHeight),
                     externalView->getThumbUrl(), QSize(1, 1));
        gif.setIsGiphy(gifUtils.isGiphyLink(link));
        qDebug() << "GIF uri:" << uri << "url:" << gif.getUrl() << "size:" << gif.getSize() << "small:" << gif.getSmallUrl() << "size:" << gif.getSmallSize() << "image:" << gif.getImageUrl();
        data->setGif(gif);
    }
    else
    {
        data->setExternalLink(link);
    }
}

static void setLabels(DraftPostData* data, const Post& post)
{
    QStringList labelNames;
    const auto labels = ContentFilter::getContentLabels(post.getLabels());

    for (const auto& label : labels)
        labelNames.push_back(label.getLabelId());

    data->setLabels(labelNames);
}

void DraftPosts::setReplyRestrictions(DraftPostData* data, const Post& post)
{
    const auto restriction = post.getReplyRestriction();

    if (restriction == QEnums::REPLY_RESTRICTION_NONE)
        return;

    data->setRestrictReplies(true);

    if (restriction & QEnums::REPLY_RESTRICTION_NOBODY)
        return;

    data->setAllowMention(restriction & QEnums::REPLY_RESTRICTION_MENTIONED);
    data->setAllowFollower(restriction & QEnums::REPLY_RESTRICTION_FOLLOWER);
    data->setAllowFollowing(restriction & QEnums::REPLY_RESTRICTION_FOLLOWING);

    if (restriction & QEnums::REPLY_RESTRICTION_LIST)
    {
        QStringList allowLists;
        const auto lists = post.getReplyRestrictionLists();

        for (const auto& list : lists)
            allowLists.push_back(list.getUri());

        data->setAllowLists(allowLists);
    }
}

// NOTE: this is also called for editing post (class PostEditUtils)
// Setting images, gif and video will fail in that case.
void DraftPosts::setDraftPost(DraftPostData* data, const Post& post)
{
    data->setText(post.getText());
    data->setEmbeddedLinks(post.getDraftEmbeddedLinks());
    setImages(data, post.getDraftImages());

    const auto videoView = post.getDraftVideoView();
    if (videoView)
        setVideo(data, *videoView);

    data->setIndexedAt(post.getIndexedAt());
    data->setReplyToUri(post.getReplyToUri());
    data->setReplyToCid(post.getReplyToCid());
    data->setReplyRootUri(post.getReplyRootUri());
    data->setReplyRootCid(post.getReplyRootCid());

    const auto replyToAuthor = post.getReplyToAuthor();
    if (replyToAuthor)
        data->setReplyToAuthor(*replyToAuthor);

    const auto replyView = post.getViewPostReplyRef();
    if (replyView)
    {
        data->setReplyToText(replyView->mParent.getText());
        data->setReplyToDateTime(replyView->mParent.getIndexedAt());
    }

    const auto recordView = post.getRecordView();
    if (recordView)
        setRecordViewData(data, recordView.get());

    const auto recordMediaView = post.getRecordWithMediaView();
    if (recordMediaView)
    {
        setImages(data, recordMediaView->getImages());

        const auto video = recordMediaView->getVideo();
        if (!video.isNull())
            setVideo(data, video.value<VideoView>());

        setRecordViewData(data, &recordMediaView->getRecord());
    }

    const auto externalView = post.getExternalView();
    if (externalView)
        setExternal(data, externalView.get());

    if (post.hasLanguage())
        data->setLanguage(post.getLanguages().front().getShortCode());

    setLabels(data, post);
    setReplyRestrictions(data, post);

    data->setEmbeddingDisabled(post.isEmbeddingDisabled());
    data->setRecordUri(post.getUri());
}

QList<DraftPostData*> DraftPosts::getDraftPostData(int index)
{
    getDraftPostsModel();

    if (index < 0 || index >= mDraftPostsModel->rowCount())
    {
        qWarning() << "Invalid index:" << index << "count:" << mDraftPostsModel->rowCount();
        return {};
    }

    QList<DraftPostData*> draftPostData;
    const std::vector<Post>& thread = mDraftPostsModel->getThread(index);

    for (const auto& post : thread)
    {
        auto* data = new DraftPostData(this);
        setDraftPost(data, post);
        draftPostData.push_back(data);
    }

    return draftPostData;
}

void DraftPosts::removeDraftPost(int index)
{
    getDraftPostsModel();

    if (index < 0 || index >= mDraftPostsModel->rowCount())
    {
        qWarning() << "Invalid index:" << index << "count:" << mDraftPostsModel->rowCount();
        return;
    }

    const Post& post = mDraftPostsModel->getPost(index);

    // NOTE:
    // With file storage, the record-uri is the file name
    // With Bluesky storage, the record-uri is the draft ID
    const QString& recordUri = post.getUri();
    qDebug() << "Remove draft post:" << index << "uri:" << recordUri;

    switch(mStorageType)
    {
    case STORAGE_FILE:
        dropDraftPost(recordUri);
        mDraftPostsModel->deleteDraft(index);
        break;
    case STORAGE_BLUESKY:
        deleteBlueskyDraft(recordUri, index);
        break;
    }
}

QString DraftPosts::getMediaStorageWarning(int index)
{
    getDraftPostsModel();
    return mDraftPostsModel->getStoredMediaWarning(index);
}

void DraftPosts::removeDraftPostsModel()
{
    mDraftPostsModel = nullptr;
}

QString DraftPosts::getDraftUri(const QString& ref) const
{
    const QString userDid = mSkywalker->getUserDid();
    ATProto::ATUri atUri(userDid, Lexicon::COLLECTION_DRAFT_POST, ref);
    return atUri.toString();
}

QString DraftPosts::getDraftsPath() const
{
    const auto did = mSkywalker->getUserDid();
    const auto subDir = QString("%1/%2").arg(did, DRAFT_POSTS_DIR);
    const QString draftsPath = FileUtils::getAppDataPath(subDir);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << subDir;
        return {};
    }

    return draftsPath;
}

QString DraftPosts::getPictureDraftsPath() const
{
    const QString path = mStorageType == STORAGE_BLUESKY ? DRAFT_BSKY_PICTURES_DIR : DRAFT_PICTURES_DIR;
    const QString draftsPath = FileUtils::getPicturesPath(path);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << path;
        return {};
    }

    return draftsPath;
}

QString DraftPosts::createDraftPostFileName(const QString& baseName) const
{
    return QString("SWP_%1.json").arg(baseName);
}

QString DraftPosts::createDraftImageFileName(const QString& baseName, int seq) const
{
    return QString("SWI%1_%2.jpg").arg(seq).arg(baseName);
}

QString DraftPosts::createDraftVideoFileName(const QString& baseName) const
{
    return QString("SWV_%1.mp4").arg(baseName);
}

QString DraftPosts::getBaseNameFromPostFileName(const QString& fileName) const
{
    auto parts = fileName.split('_');
    if (parts.size() != 2)
        return {};

    parts = parts[1].split('.');
    if (parts.size() != 2)
        return {};

    qDebug() << "Base name from:" << fileName << "=" << parts[0];
    return parts[0];
}

QString DraftPosts::getBaseNameFromMediaFullFileUrl(const QString& fileUrl) const
{
    if (!fileUrl.startsWith("file://"))
    {
        qWarning() << "Not a file URL:" << fileUrl;
        return {};
    }

    const QFileInfo fileInfo(fileUrl.sliced(7));
    const QString base = fileInfo.baseName();

    // Example base: SWI1_20251226131809-0
    auto parts = base.split('_');
    if (parts.size() != 2)
        return {};

    parts = parts[1].split('-');
    if (parts.size() != 2)
        return {};

    qDebug() << "Base name from:" << fileUrl << "=" << parts[0];
    return parts[0];
}

ATProto::AppBskyActor::ProfileViewBasic::SharedPtr DraftPosts::createProfileViewBasic(const BasicProfile& author)
{
    if (author.isNull())
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyActor::ProfileViewBasic>();
    view->mDid = author.getDid();
    view->mHandle = author.getHandle();
    view->mDisplayName = author.getDisplayName();
    view->mAvatar = author.getAvatarUrl();

    return view;
}

ATProto::AppBskyActor::ProfileView::SharedPtr DraftPosts::createProfileView(const Profile& author)
{
    if (author.isNull())
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyActor::ProfileView>();
    view->mDid = author.getDid();
    view->mHandle = author.getHandle();
    view->mDisplayName = author.getDisplayName();
    view->mAvatar = author.getAvatarUrl();

    return view;
}

Draft::EmbeddedLinks::SharedPtr DraftPosts::createEmbeddedLinks(const WebLink::List& links)
{
    auto embeddedLinks = std::make_shared<Draft::EmbeddedLinks>();
    embeddedLinks->mEmbeddedLinks.reserve(links.size());

    for (const auto& link : links)
    {
        auto draftLink = std::make_shared<WebLink>(link);
        embeddedLinks->mEmbeddedLinks.push_back(draftLink);
    }

    return embeddedLinks;
}

Draft::ReplyToPost::SharedPtr DraftPosts::createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                               const QString& text, const QDateTime& dateTime) const
{
    if (replyToUri.isEmpty())
        return nullptr;

    auto replyToPost = std::make_shared<Draft::ReplyToPost>();
    replyToPost->mAuthor = createProfileViewBasic(author);
    replyToPost->mText = text;
    replyToPost->mDateTime = dateTime;
    return replyToPost;
}

Draft::Quote::SharedPtr DraftPosts::createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
                                   const QString& quoteText, const QDateTime& quoteDateTime,
                                   const GeneratorView& quoteFeed, const ListView& quoteList) const
{
    if (quoteUri.isEmpty())
        return nullptr;

    const ATProto::ATUri atUri(quoteUri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid quote uri:" << quoteUri;
        return nullptr;
    }

    auto quote = std::make_shared<Draft::Quote>();

    if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_POST)
    {
        quote->mRecordType = Draft::Quote::RecordType::QUOTE_POST;
        quote->mRecord = createQuotePost(quoteAuthor, quoteText, quoteDateTime);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_GENERATOR)
    {
        quote->mRecordType = Draft::Quote::RecordType::QUOTE_FEED;
        quote->mRecord = createQuoteFeed(quoteFeed);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_GRAPH_LIST)
    {
        quote->mRecordType = Draft::Quote::RecordType::QUOTE_LIST;
        quote->mRecord = createQuoteList(quoteList);
    }
    else
    {
        qWarning() << "Unknown quote type:" << quoteUri;
        return nullptr;
    }

    return quote;
}

Draft::QuotePost::SharedPtr DraftPosts::createQuotePost(const BasicProfile& author,
                                           const QString& text, const QDateTime& dateTime) const
{
    auto quotePost = std::make_shared<Draft::QuotePost>();
    quotePost->mAuthor = createProfileViewBasic(author);
    quotePost->mText = text;
    quotePost->mDateTime = dateTime;
    return quotePost;
}

ATProto::AppBskyFeed::GeneratorView::SharedPtr DraftPosts::createQuoteFeed(const GeneratorView& feed) const
{
    auto view = std::make_shared<ATProto::AppBskyFeed::GeneratorView>();
    view->mUri = feed.getUri();
    view->mCid = feed.getCid();
    view->mDid = feed.getDid();
    view->mCreator = createProfileView(feed.getCreator());
    view->mDisplayName = feed.getDisplayName();
    view->mDescription = feed.getDescription();
    view->mAvatar = feed.getAvatar();
    view->mIndexedAt = QDateTime::currentDateTimeUtc();
    return view;
}

ATProto::AppBskyGraph::ListView::SharedPtr DraftPosts::createQuoteList(const ListView& list) const
{
    auto view = std::make_shared<ATProto::AppBskyGraph::ListView>();
    view->mUri = list.getUri();
    view->mCid = list.getCid();
    view->mCreator = createProfileView(list.getCreator());
    view->mName = list.getName();
    view->mPurpose = ATProto::AppBskyGraph::ListPurpose(list.getPurpose());
    view->mDescription = list.getDescription();
    view->mAvatar = list.getAvatar();
    return view;
}

static Draft::Quote::SharedPtr getDraftQuote(const ATProto::AppBskyFeed::Record::Post::SharedPtr& post)
{
    const ATProto::XJsonObject xjson(post->mJson);
    auto quote = xjson.getOptionalObject<Draft::Quote>(Lexicon::DRAFT_POST_QUOTE_FIELD);
    return quote;
}

ATProto::AppBskyFeed::PostFeed DraftPosts::convertDraftToFeedViewPost(Draft::Draft& draft, const QString& recordUri)
{
    ATProto::AppBskyFeed::PostFeed postFeed;
    auto feedView = std::make_shared<ATProto::AppBskyFeed::FeedViewPost>();
    feedView->mReply = createReplyRef(draft.mReplyToPost, draft.mPost->mReply);
    feedView->mPost = convertDraftToPostView(draft, recordUri);
    postFeed.push_back(std::move(feedView));

    for (auto& threadPost : draft.mThreadPosts)
    {
        Draft::Draft threadPostDraft;
        threadPostDraft.mPost = std::move(threadPost);
        threadPostDraft.mQuote = getDraftQuote(threadPostDraft.mPost);
        auto view = std::make_shared<ATProto::AppBskyFeed::FeedViewPost>();
        view->mPost = convertDraftToPostView(threadPostDraft, recordUri);
        postFeed.push_back(std::move(view));
    }

    return postFeed;
}

ATProto::AppBskyFeed::PostView::SharedPtr DraftPosts::convertDraftToPostView(Draft::Draft& draft, const QString& recordUri)
{
    auto postView = std::make_shared<ATProto::AppBskyFeed::PostView>();
    postView->mUri = recordUri;
    postView->mAuthor = createProfileViewBasic(mSkywalker->getUser());
    postView->mIndexedAt = draft.mPost->mCreatedAt;
    postView->mEmbed = createEmbedView(draft.mPost->mEmbed.get(), std::move(draft.mQuote));
    postView->mLabels = createContentLabels(*draft.mPost, recordUri);
    postView->mRecord = std::move(draft.mPost);
    postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    postView->mThreadgate = createThreadgateView(draft);
    postView->mViewer = createViewerState(draft);

    return postView;
}

ATProto::AppBskyFeed::ViewerState::SharedPtr DraftPosts::createViewerState(Draft::Draft& draft) const
{
    auto viewer = std::make_shared<ATProto::AppBskyFeed::ViewerState>();
    viewer->mEmbeddingDisabled = draft.mEmbeddingDisabled;
    return viewer;
}

ATProto::AppBskyFeed::ThreadgateView::SharedPtr DraftPosts::createThreadgateView(Draft::Draft& draft) const
{
    if (!draft.mThreadgate)
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyFeed::ThreadgateView>();
    view->mRecord = std::move(draft.mThreadgate);

    for (const auto& list : view->mRecord->mRules.mAllowList)
    {
        auto listView = std::make_shared<ATProto::AppBskyGraph::ListViewBasic>();
        listView->mUri = list->mList;
        listView->mName = list->mList;
        view->mLists.push_back(std::move(listView));
    }

    return view;
}

ATProto::ComATProtoLabel::Label::List DraftPosts::createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& recordUri) const
{
    return createContentLabels(post.mLabels, post.mCreatedAt, recordUri);
}

ATProto::ComATProtoLabel::Label::List DraftPosts::createContentLabels(const ATProto::ComATProtoLabel::SelfLabels::SharedPtr& selfLabels, QDateTime createdAt, const QString& recordUri) const
{
    if (!selfLabels)
        return {};

    const QString userDid = mSkywalker->getUserDid();
    ATProto::ComATProtoLabel::Label::List labels;

    for (const auto& selfLabel : selfLabels->mValues)
    {
        auto label = std::make_shared<ATProto::ComATProtoLabel::Label>();
        label->mVal = selfLabel->mVal;
        label->mSrc = userDid;
        label->mUri = recordUri;
        label->mCreatedAt = createdAt;
        labels.push_back(std::move(label));
    }

    return labels;
}

ATProto::AppBskyEmbed::EmbedView::SharedPtr DraftPosts::createEmbedView(
    const ATProto::AppBskyEmbed::Embed* embed, Draft::Quote::SharedPtr quote)
{
    if (!embed)
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::EmbedView>();

    switch (embed->mType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mEmbed = createImagesView(std::get<ATProto::AppBskyEmbed::Images::SharedPtr>(embed->mEmbed).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::VIDEO:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW;
        view->mEmbed = createVideoView(std::get<ATProto::AppBskyEmbed::Video::SharedPtr>(embed->mEmbed).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::EXTERNAL:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
        view->mEmbed = createExternalView(std::get<ATProto::AppBskyEmbed::External::SharedPtr>(embed->mEmbed).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::RECORD:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
        view->mEmbed = createRecordView(std::get<ATProto::AppBskyEmbed::Record::SharedPtr>(embed->mEmbed).get(), std::move(quote));
        break;
    case ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
        view->mEmbed = createRecordWithMediaView(std::get<ATProto::AppBskyEmbed::RecordWithMedia::SharedPtr>(embed->mEmbed).get(), std::move(quote));
        break;
    case ATProto::AppBskyEmbed::EmbedType::UNKNOWN:
        qWarning() << "Unknown embed type";
        break;
    }

    const bool embedSet = std::visit([](const auto& v){ return v != nullptr; }, view->mEmbed);

    if (!embedSet)
    {
        qWarning() << "Embed could not be created for:" << (int)view->mType;
        return nullptr;
    }

    return view;
}

ATProto::AppBskyEmbed::ImagesView::SharedPtr DraftPosts::createImagesView(const ATProto::AppBskyEmbed::Images* images)
{
    if (!images || images->mImages.empty())
        return nullptr;

    const QString did = mSkywalker->getUserDid();
    auto view = std::make_shared<ATProto::AppBskyEmbed::ImagesView>();

    for (const auto& image : images->mImages)
    {
        const auto draftsPath = getPictureDraftsPath();

        if (draftsPath.isEmpty())
            return nullptr;

        const QString path = createAbsPath(draftsPath, image->mImage->mRefLink);
        const QString imgSource = "file://" + path;

        auto imgView = createImageView(image->mImage->mJson, imgSource, image->mAlt);
        view->mImages.push_back(std::move(imgView));
    }

    return view;
}

ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr DraftPosts::createImageView(const QJsonObject& imgJson, const QString& imgSource, const QString& alt)
{
    const ATProto::XJsonObject xjson(imgJson);
    const QString memeTopText = xjson.getOptionalString(Lexicon::DRAFT_MEME_TOP_TEXT_FIELD, "");
    const QString memeBottomText = xjson.getOptionalString(Lexicon::DRAFT_MEME_BOTTOM_TEXT_FIELD, "");

    auto imgView = std::make_shared<ATProto::AppBskyEmbed::ImagesViewImage>();
    imgView->mThumb = imgSource;
    imgView->mFullSize = imgSource;
    imgView->mAlt = alt;

    if (!memeTopText.isEmpty())
        imgView->mJson.insert(Lexicon::DRAFT_MEME_TOP_TEXT_FIELD, memeTopText);

    if (!memeBottomText.isEmpty())
        imgView->mJson.insert(Lexicon::DRAFT_MEME_BOTTOM_TEXT_FIELD, memeBottomText);

    return imgView;
}

ATProto::AppBskyEmbed::VideoView::SharedPtr DraftPosts::createVideoView(const ATProto::AppBskyEmbed::Video* video)
{
    if (!video || !video->mVideo)
        return nullptr;

    const auto draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
        return nullptr;

    const QString path = createAbsPath(draftsPath, video->mVideo->mRefLink);
    const QString videoSource = "file://" + path;

    return createVideoView(video->mVideo->mJson, videoSource, video->mAlt, video->mPresentation);
}

ATProto::AppBskyEmbed::VideoView::SharedPtr DraftPosts::createVideoView(const QJsonObject& videoJson, const QString& videoSource, const std::optional<QString>& alt, const std::optional<ATProto::AppBskyEmbed::VideoPresentation>& presentation)
{
    const ATProto::XJsonObject xjson(videoJson);
    const int startMs = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_START_MS_FIELD, 0);
    const int endMs = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_END_MS_FIELD, 0);
    const bool removeAudio = xjson.getOptionalBool(Lexicon::DRAFT_VIDEO_REMOVE_AUDIO_FIELD, false);
    const int newHeight = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_NEW_HEIGHT_FIELD, 0);

    auto view = std::make_shared<ATProto::AppBskyEmbed::VideoView>();
    view->mPlaylist = videoSource;
    view->mAlt = alt;
    view->mPresentation = presentation;

    if (startMs > 0)
        view->mJson.insert(Lexicon::DRAFT_VIDEO_START_MS_FIELD, startMs);

    if (endMs > 0)
        view->mJson.insert(Lexicon::DRAFT_VIDEO_END_MS_FIELD, endMs);

    if (removeAudio)
        view->mJson.insert(Lexicon::DRAFT_VIDEO_REMOVE_AUDIO_FIELD, removeAudio);

    if (newHeight > 0)
        view->mJson.insert(Lexicon::DRAFT_VIDEO_NEW_HEIGHT_FIELD, newHeight);

    return view;
}

ATProto::AppBskyEmbed::ExternalView::SharedPtr DraftPosts::createExternalView(
    const ATProto::AppBskyEmbed::External* external) const
{
    if (!external || !external->mExternal)
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::ExternalView>();
    view->mExternal = std::make_shared<ATProto::AppBskyEmbed::ExternalViewExternal>();
    // NOTE: the small gif size and url are encoded as URL params to this uri.
    view->mExternal->mUri = external->mExternal->mUri;
    view->mExternal->mTitle = external->mExternal->mTitle;
    view->mExternal->mDescription = external->mExternal->mDescription;

    const QUrl url(external->mExternal->mUri);
    const QUrlQuery query(url.query());
    view->mExternal->mThumb = query.queryItemValue("imageUrl");

    return view;
}

ATProto::AppBskyEmbed::RecordView::SharedPtr DraftPosts::createRecordView(
    const ATProto::AppBskyEmbed::Record* record, Draft::Quote::SharedPtr quote) const
{
    if (!quote)
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordView>();

    switch (quote->mRecordType)
    {
    case Draft::Quote::RecordType::QUOTE_POST:
    {
        Q_ASSERT(record);

        if (!record)
        {
            qWarning() << "Need record for a quote";
            return nullptr;
        }

        auto& quotePost = std::get<Draft::QuotePost::SharedPtr>(quote->mRecord);
        auto post = std::make_shared<ATProto::AppBskyFeed::Record::Post>();
        post->mText = quotePost->mText;
        post->mCreatedAt = quotePost->mDateTime;

        auto viewRecord = std::make_shared<ATProto::AppBskyEmbed::RecordViewRecord>();
        viewRecord->mUri = record->mRecord->mUri;
        viewRecord->mCid = record->mRecord->mCid;
        viewRecord->mAuthor = std::move(quotePost->mAuthor);
        viewRecord->mValue = std::move(post);
        viewRecord->mValueType = ATProto::RecordType::APP_BSKY_FEED_POST;
        viewRecord->mIndexedAt = quotePost->mDateTime;

        view->mRecordType = ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD;
        view->mRecord = std::move(viewRecord);
        break;
    }
    case Draft::Quote::RecordType::QUOTE_FEED:
        view->mRecordType = ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW;
        view->mRecord = std::move(std::get<ATProto::AppBskyFeed::GeneratorView::SharedPtr>(quote->mRecord));
        break;
    case Draft::Quote::RecordType::QUOTE_LIST:
        view->mRecordType = ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW;
        view->mRecord = std::move(std::get<ATProto::AppBskyGraph::ListView::SharedPtr>(quote->mRecord));
        break;
    case Draft::Quote::RecordType::QUOTE_LABELER:
        view->mRecordType = ATProto::RecordType::APP_BSKY_LABELER_VIEW;
        view->mRecord = std::move(std::get<ATProto::AppBskyLabeler::LabelerView::SharedPtr>(quote->mRecord));
        break;
    case Draft::Quote::RecordType::QUOTE_STARTER_PACK:
        view->mRecordType = ATProto::RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC;
        view->mRecord = std::move(std::get<ATProto::AppBskyGraph::StarterPackViewBasic::SharedPtr>(quote->mRecord));
        break;
    default:
        qWarning() << "Unknown record type" << (int)quote->mRecordType;
        view->mRecordType = ATProto::RecordType::UNKNOWN;
        view->mUnsupportedType = QString::number((int)quote->mRecordType);
        break;
    }

    return view;
}

ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr DraftPosts::createRecordWithMediaView(
    const ATProto::AppBskyEmbed::RecordWithMedia* record, Draft::Quote::SharedPtr quote)
{
    if (!record || !quote)
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record->mRecord.get(), std::move(quote));

    switch (record->mMediaType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mMedia = createImagesView(std::get<ATProto::AppBskyEmbed::Images::SharedPtr>(record->mMedia).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::EXTERNAL:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
        view->mMedia = createExternalView(std::get<ATProto::AppBskyEmbed::External::SharedPtr>(record->mMedia).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::VIDEO:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW;
        view->mMedia = createVideoView(std::get<ATProto::AppBskyEmbed::Video::SharedPtr>(record->mMedia).get());
        break;
    default:
        qWarning() << "Invalid media type:" << (int)record->mMediaType;
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::UNKNOWN;
        view->mRawMediaType = QString::number((int)record->mMediaType);
        break;
    }

    return view;
}

// The obvious test is the check deviceId in the draft against this deviceId.
// However this is not reliable. When the user re-installs the app, a new deviceId will
// be generated, while the media files from the draft are still on this device.
// When the user copies all contents of a device to a new device, then both devices will
// have the same deviceId.
// Instead we check the media file names from the posts to find out if the files exist.
QString DraftPosts::checkMediaStorage(const ATProto::AppBskyDraft::DraftView& draft, const ATProto::AppBskyFeed::PostFeed& postFeed, int embedImagesCount, int embedVideoCount) const
{
    qDebug() << "Check media storage:" << draft.mId << "embedImagesCount:" << embedImagesCount << "embedVideoCount:" << embedVideoCount;

    if (embedImagesCount + embedVideoCount == 0)
        return {};

    std::vector<Post> thread;

    for (const auto& feedViewPost : postFeed)
        thread.push_back(Post(feedViewPost));

    const QString mediaBaseName = getMediaBaseName(thread);

    if (!mediaBaseName.isEmpty())
    {
        const QString mediaPath = getPictureDraftsPath();

        if (mediaPath.isEmpty())
        {
            qWarning() << "Cannot get media path";
            return {};
        }

        QDir mediaDir(mediaPath);
        const QString filter = QString("SW*_%1-*").arg(mediaBaseName);
        const auto mediaFileNames = mediaDir.entryList({ filter }, QDir::Files);

        if (!mediaFileNames.empty())
        {
            qDebug() << "Media files found:" << draft.mId << "files:" << mediaFileNames;
            return {};
        }
    }
    else
    {
        qDebug() << "Media files probably written by another app:" << draft.mId;
    }

    QString warning;

    const QString draftDeviceName = draft.mDraft->mDeviceName.value_or(tr("other device"));
    const QString thisDeviceName = FileUtils::getDeviceName();
    const QString otherDeviceName = draftDeviceName == thisDeviceName ? tr("%1 (other app)").arg(draftDeviceName) : draftDeviceName;

    const QString imageWord = embedImagesCount == 1 ? tr("image") : tr("images");
    const QString videoWord = embedVideoCount == 1 ? tr("video") : tr("videos");

    // HACK: abusing feed context to show this warning in the drafts overview
    if (embedVideoCount == 0)
        warning = tr("%1 %2 stored on %3").arg(embedImagesCount).arg(imageWord, otherDeviceName);
    else if (embedImagesCount == 0)
        warning = tr("%1 %2 stored on %3").arg(embedVideoCount).arg(videoWord, otherDeviceName);
    else
        warning = tr("%1 %2 and %3 %4 on %5").arg(embedImagesCount).arg(imageWord).arg(embedVideoCount).arg(videoWord, otherDeviceName);

    return warning;
}

ATProto::AppBskyFeed::PostFeed DraftPosts::convertDraftToFeedViewPost(const ATProto::AppBskyDraft::DraftView& draft, const QString& recordUri)
{
    ATProto::AppBskyFeed::PostFeed postFeed;
    int embedImagesCount = 0;
    int embedVideoCount = 0;

    for (const auto& threadPost : draft.mDraft->mPosts)
    {
        auto view = std::make_shared<ATProto::AppBskyFeed::FeedViewPost>();
        view->mPost = convertDraftToPostView(draft, *threadPost, recordUri);

        ATProto::XJsonObject xjson(draft.mDraft->mJson);
        auto replyTo = xjson.getOptionalObject<Draft::ReplyToPost>(Lexicon::DRAFT_REPPLY_TO);
        auto postRecord = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(view->mPost->mRecord);
        view->mReply = createReplyRef(replyTo, postRecord->mReply);

        embedImagesCount += threadPost->mEmbedImages.size();
        embedVideoCount += threadPost->mEmbedVideos.size();
        postFeed.push_back(std::move(view));
    }

    const QString mediaWarning = checkMediaStorage(draft, postFeed, embedImagesCount, embedVideoCount);

    if (!mediaWarning.isEmpty())
    {
        // HACK: abusing feed context to show this warning in the drafts overview
        auto& view = postFeed.front();
        view->mFeedContext = mediaWarning;
    }

    return postFeed;
}

ATProto::AppBskyFeed::ReplyRef::SharedPtr DraftPosts::createReplyRef(const Draft::ReplyToPost::SharedPtr& replyToPost, const ATProto::AppBskyFeed::PostReplyRef::SharedPtr& postReplyRef) const
{
    if (!replyToPost || !postReplyRef)
        return nullptr;

    Q_ASSERT(postReplyRef->mRoot);

    if (!postReplyRef->mRoot)
    {
        qWarning() << "Reply root information missing from draft for reply:" << replyToPost->mText;
        return nullptr;
    }

    auto replyRef = std::make_shared<ATProto::AppBskyFeed::ReplyRef>();
    replyRef->mParent = std::make_shared<ATProto::AppBskyFeed::ReplyElement>();
    replyRef->mParent->mType = ATProto::AppBskyFeed::PostElementType::POST_VIEW;
    replyRef->mParent->mPost = convertReplyToPostView(replyToPost, postReplyRef);

    // We did not save the root post in the draft. Set it to NOT FOUND. The post composer
    // does not need it.
    replyRef->mRoot = std::make_shared<ATProto::AppBskyFeed::ReplyElement>();
    replyRef->mRoot->mType = ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST;
    auto notFound = std::make_shared<ATProto::AppBskyFeed::NotFoundPost>();
    notFound->mUri = postReplyRef->mRoot->mUri;
    replyRef->mRoot->mPost = std::move(notFound);

    return replyRef;
}

ATProto::AppBskyFeed::PostView::SharedPtr DraftPosts::convertReplyToPostView(const Draft::ReplyToPost::SharedPtr& replyToPost, const ATProto::AppBskyFeed::PostReplyRef::SharedPtr& postReplyRef) const
{
    if (!replyToPost)
        return nullptr;

    Q_ASSERT(postReplyRef);
    Q_ASSERT(postReplyRef->mParent);

    if (!postReplyRef || !postReplyRef->mParent)
    {
        qWarning() << "Reply parent information missing from draft for reply:" << replyToPost->mText;
        return nullptr;
    }

    auto view = std::make_shared<ATProto::AppBskyFeed::PostView>();
    view->mUri = postReplyRef->mParent->mUri;
    view->mCid = postReplyRef->mParent->mCid;
    view->mAuthor = std::move(replyToPost->mAuthor);
    view->mRecord = createReplyToPost(replyToPost);
    view->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    view->mIndexedAt = replyToPost->mDateTime;
    return view;
}

ATProto::AppBskyFeed::Record::Post::SharedPtr DraftPosts::createReplyToPost(const Draft::ReplyToPost::SharedPtr& replyToPost) const
{
    auto post = std::make_shared<ATProto::AppBskyFeed::Record::Post>();
    post->mText = replyToPost->mText;
    post->mCreatedAt = replyToPost->mDateTime;
    return post;
}

ATProto::AppBskyFeed::PostView::SharedPtr DraftPosts::convertDraftToPostView(const ATProto::AppBskyDraft::DraftView& draftView, const ATProto::AppBskyDraft::DraftPost& draftPost, const QString& recordUri)
{
    auto postView = std::make_shared<ATProto::AppBskyFeed::PostView>();
    postView->mUri = recordUri;
    postView->mAuthor = createProfileViewBasic(mSkywalker->getUser());
    postView->mIndexedAt = draftView.mUpdatedAt;
    postView->mEmbed = createEmbedView(draftPost);
    postView->mLabels = createContentLabels(draftPost.mLabels, draftView.mCreatedAt, recordUri);

    auto postRecord = std::make_shared<ATProto::AppBskyFeed::Record::Post>();
    postRecord->mText = draftPost.mText;
    postRecord->mLabels = draftPost.mLabels;
    postRecord->mLanguages = draftView.mDraft->mLangs;
    postRecord->mCreatedAt = draftView.mCreatedAt;

    ATProto::XJsonObject xjson(draftPost.mJson);
    postRecord->mReply = xjson.getOptionalObject<ATProto::AppBskyFeed::PostReplyRef>(Lexicon::DRAFT_REPPLY_REF);
    std::optional<QJsonObject> embeddedLinks = xjson.getOptionalJsonObject(Lexicon::DRAFT_EMBBEDED_LINKS_FIELD);

    if (embeddedLinks)
        postRecord->mJson.insert(Lexicon::DRAFT_EMBBEDED_LINKS_FIELD, *embeddedLinks);

    postView->mRecord = postRecord;
    postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    postView->mThreadgate = createThreadgateView(draftView.mDraft->mThreadgateRules, recordUri, draftView.mCreatedAt);
    postView->mViewer = createViewerState(*draftView.mDraft);

    return postView;
}

ATProto::AppBskyFeed::ViewerState::SharedPtr DraftPosts::createViewerState(const ATProto::AppBskyDraft::Draft& draft) const
{
    auto viewer = std::make_shared<ATProto::AppBskyFeed::ViewerState>();
    viewer->mEmbeddingDisabled = draft.mDisableEmbedding;
    return viewer;
}

ATProto::AppBskyFeed::ThreadgateView::SharedPtr DraftPosts::createThreadgateView(const ATProto::AppBskyFeed::ThreadgateRules& threadgateRules, const QString& recordUri, QDateTime createdAt) const
{
    if (!threadgateRules.mAllowNobody && !threadgateRules.mAllowFollower && !threadgateRules.mAllowFollowing && !threadgateRules.mAllowFollower && threadgateRules.mAllowList.empty())
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyFeed::ThreadgateView>();
    view->mRecord = std::make_shared<ATProto::AppBskyFeed::Threadgate>();
    view->mRecord->mRules = threadgateRules;
    view->mRecord->mPost = recordUri;
    view->mRecord->mCreatedAt = createdAt;

    for (const auto& list : view->mRecord->mRules.mAllowList)
    {
        auto listView = std::make_shared<ATProto::AppBskyGraph::ListViewBasic>();
        listView->mUri = list->mList;
        listView->mName = list->mList;
        view->mLists.push_back(std::move(listView));
    }

    return view;
}

bool DraftPosts::hasEmbed(const ATProto::AppBskyDraft::DraftPost& draftPost) const
{
    return !draftPost.mEmbedRecords.empty() ||
           !draftPost.mEmbedExternals.empty() ||
           hasMediaEmbed(draftPost);
}

bool DraftPosts::hasMediaEmbed(const ATProto::AppBskyDraft::DraftPost& draftPost) const
{
    return !draftPost.mEmbedImages.empty() ||
           !draftPost.mEmbedVideos.empty();
}

ATProto::AppBskyEmbed::EmbedView::SharedPtr DraftPosts::createEmbedView(const ATProto::AppBskyDraft::DraftPost& draftPost)
{
    if (!hasEmbed(draftPost))
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::EmbedView>();

    if (!draftPost.mEmbedRecords.empty())
    {
        if (!draftPost.mEmbedImages.empty())
        {
            auto imagesView = createImagesView(draftPost.mEmbedImages);

            if (imagesView)
            {
                view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
                view->mEmbed = createRecordWithMediaView(imagesView, *draftPost.mEmbedRecords.front());
            }
            else
            {
                qDebug() << "Failed to create images view";
                view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
                view->mEmbed = createRecordView(*draftPost.mEmbedRecords.front());
            }
        }
        else if (!draftPost.mEmbedVideos.empty())
        {
            auto videoView = createVideoView(*draftPost.mEmbedVideos.front());

            if (videoView)
            {
                view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
                view->mEmbed = createRecordWithMediaView(videoView, *draftPost.mEmbedRecords.front());
            }
            else
            {
                qDebug() << "Failed to create video view";
                view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
                view->mEmbed = createRecordView(*draftPost.mEmbedRecords.front());
            }
        }
        else if (!draftPost.mEmbedExternals.empty())
        {
            auto externalView = createExternalView(*draftPost.mEmbedExternals.front());
            view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
            view->mEmbed = createRecordWithMediaView(externalView, *draftPost.mEmbedRecords.front());
        }
        else
        {
            view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
            view->mEmbed = createRecordView(*draftPost.mEmbedRecords.front());
        }
    }
    else if (!draftPost.mEmbedImages.empty())
    {
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mEmbed = createImagesView(draftPost.mEmbedImages);
    }
    else if (!draftPost.mEmbedVideos.empty())
    {
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW;
        view->mEmbed = createVideoView(*draftPost.mEmbedVideos.front());
    }
    else if (!draftPost.mEmbedExternals.empty())
    {
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
        view->mEmbed = createExternalView(*draftPost.mEmbedExternals.front());
    }
    else
    {
        return nullptr;
    }

    const bool embedSet = std::visit([](auto&& v){ return v != nullptr; }, view->mEmbed);

    if (!embedSet)
    {
        qDebug() << "Embed could not be created:" << (int)view->mType;
        return nullptr;
    }

    return view;
}

ATProto::AppBskyEmbed::ImagesView::SharedPtr DraftPosts::createImagesView(const ATProto::AppBskyDraft::DraftEmbedImage::List& images)
{
    const auto draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
        return nullptr;

    auto view = std::make_shared<ATProto::AppBskyEmbed::ImagesView>();

    for (const auto& image : images)
    {
        const QString path = createAbsPath(draftsPath, image->mLocalRef->mPath);

        // We explicitly test if the image exists on this device. Checking the deviceId is not
        // reliable. deviceId changes on re-install, or may be copied to another device.
        if (QFile::exists(path))
        {
            const QString imgSource = "file://" + path;
            auto imgView = createImageView(image->mJson, imgSource, image->mAlt.value_or(""));
            view->mImages.push_back(std::move(imgView));
        }
        else
        {
            qDebug() << "File does not exist:" << path;
        }
    }

    if (view->mImages.empty())
        return nullptr;

    return view;
}

ATProto::AppBskyEmbed::VideoView::SharedPtr DraftPosts::createVideoView(const ATProto::AppBskyDraft::DraftEmbedVideo& video)
{
    const auto draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
        return nullptr;

    const QString path = createAbsPath(draftsPath, video.mLocalRef->mPath);

    // We explicitly test if the video exists on this device. Checking the deviceId is not
    // reliable. deviceId changes on re-install, or may be copied to another device.
    if (!QFile::exists(path))
    {
        qDebug() << "File does not exist:" << path;
        return nullptr;
    }

    ATProto::XJsonObject xjson(video.mJson);
    std::optional<QString> rawPresentation = xjson.getOptionalString(Lexicon::DRAFT_VIDEO_PRESENTATION);
    std::optional<ATProto::AppBskyEmbed::VideoPresentation> presentation;

    if (rawPresentation)
        presentation = ATProto::AppBskyEmbed::stringToVideoPresentation(*rawPresentation);

    const QString videoSource = "file://" + path;
    return createVideoView(video.mJson, videoSource, video.mAlt, presentation);
}

ATProto::AppBskyEmbed::ExternalView::SharedPtr DraftPosts::createExternalView(const ATProto::AppBskyDraft::DraftEmbedExternal& external)
{
    auto view = std::make_shared<ATProto::AppBskyEmbed::ExternalView>();
    view->mExternal = std::make_shared<ATProto::AppBskyEmbed::ExternalViewExternal>();
    view->mExternal->mUri = external.mUri;
    return view;
}

ATProto::AppBskyEmbed::RecordView::SharedPtr DraftPosts::createRecordView(const ATProto::AppBskyDraft::DraftEmbedRecord& record)
{
    // HACK: we create a NotFound record as we do not have the full record view here.
    // In DraftsPostModel, the URI from the NotFound record will be resolved to a post.
    auto postRecord = std::make_shared<ATProto::AppBskyEmbed::RecordViewNotFound>();
    postRecord->mUri = record.mRecord->mUri;

    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordView>();
    view->mRecord = postRecord;
    view->mRecordType = ATProto::RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND;

    return view;
}

ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr DraftPosts::createRecordWithMediaView(const ATProto::AppBskyEmbed::ImagesView::SharedPtr imagesView, const ATProto::AppBskyDraft::DraftEmbedRecord& record)
{   
    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record);
    view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
    view->mMedia = imagesView;
    return view;
}

ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr DraftPosts::createRecordWithMediaView(const ATProto::AppBskyEmbed::VideoView::SharedPtr videoView, const ATProto::AppBskyDraft::DraftEmbedRecord& record)
{
    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record);
    view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW;
    view->mMedia = videoView;
    return view;
}

ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr DraftPosts::createRecordWithMediaView(const ATProto::AppBskyEmbed::ExternalView::SharedPtr externalView, const ATProto::AppBskyDraft::DraftEmbedRecord& record)
{
    auto view = std::make_shared<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record);
    view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
    view->mMedia = externalView;
    return view;
}

QString DraftPosts::dumpDraftFeed()
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
        return "No drafts path";

    QDir draftsDir(draftsPath);
    QString draftFeed;
    const auto fileList = getDraftPostFiles(draftsPath);
    draftFeed.append(QString("#drafts: %1\n").arg(fileList.size()));

    for (const auto& fileName : fileList)
    {
        draftFeed.append(QString("\n%1\n").arg(fileName));
        const QString absFileName = draftsDir.absoluteFilePath(fileName);
        QFile file(absFileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            draftFeed.append(QString("Cannot open file: %1\n").arg(absFileName));
            continue;
        }

        const QByteArray data = file.readAll();
        draftFeed.append(QString(data));
        draftFeed.append("\n");
    }

    return draftFeed;
}

void DraftPosts::loadDraftFeed()
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
        return;

    getDraftPostsModel();

    const auto fileList = getDraftPostFiles(draftsPath);
    std::vector<ATProto::AppBskyFeed::PostFeed> postThreads;

    for (const auto& file : fileList)
    {
        auto draft = loadDraft(file, draftsPath);

        if (draft)
        {
            auto postFeed = convertDraftToFeedViewPost(*draft, getDraftUri(file));
            postThreads.push_back(std::move(postFeed));
        }
        else
        {
            dropDraftPostFiles(draftsPath, file);
            const QString picDraftsPath = getPictureDraftsPath();

            if (!picDraftsPath.isEmpty())
                dropDraftPostFiles(picDraftsPath, file);
        }
    }

    mDraftPostsModel->setFeed(std::move(postThreads), {});
    emit draftsChanged();
    emit loadDraftPostsOk();
}

QStringList DraftPosts::getDraftPostFiles(const QString& draftsPath) const
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    QDir dir(draftsPath);
    return dir.entryList({"SWP_*.json"}, QDir::Files, QDir::Time);
}

Draft::Draft::SharedPtr DraftPosts::loadDraft(const QString& fileName, const QString& draftsPath) const
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    QDir dir(draftsPath);
    const QString absFileName = dir.absoluteFilePath(fileName);
    QFile file(absFileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file:" << absFileName;
        return nullptr;
    }

    const QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull())
    {
        qWarning() << "Not valid JSON:" << absFileName;
        return nullptr;
    }

    // If the user does not give permission, then we can still continue loading
    // the draft, only attached images will not be shown.
    FileUtils::checkReadMediaPermission();

    try {
        auto draft = Draft::Draft::fromJson(doc.object());
        return draft;
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << "File format error:" << e.msg();
        qInfo() << doc.object();
        return nullptr;
    }
}

bool DraftPosts::save(const Draft::Draft& draft, const QString& draftsPath, const QString& baseName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString postFileName = createDraftPostFileName(baseName);
    const QString fileName = createAbsPath(draftsPath, postFileName);
    qDebug() << "Draft post name:" << fileName;
    QFile file(fileName);

    if (file.exists())
    {
        qWarning() << "File already exists:" << fileName;
        emit saveDraftPostFailed(tr("File already exists: %1").arg(fileName));
        return true; // HACK: if a draft gets migrates twice, it was written already.
    }

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot create file:" << fileName;
        emit saveDraftPostFailed(tr("Cannot create file: %1").arg(fileName));
        return false;
    }

    const auto jsonDraft = QJsonDocument(draft.toJson());
    const QByteArray data = jsonDraft.toJson(QJsonDocument::Compact);

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

bool DraftPosts::addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                                 const QList<ImageView>& images,
                                 const QString& draftsPath, const QString& baseName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    for (int i = 0; i < images.size(); ++i)
    {
        const QString& imgName = images[i].getFullSizeUrl();
        const QString& memeTopText = images[i].getMemeTopText();
        const QString& memeBottomText = images[i].getMemeBottomText();
        auto [blob, imgSize] = saveImage(imgName, memeTopText, memeBottomText, draftsPath, baseName, i);

        if (!blob)
        {
            dropImages(draftsPath, baseName, i);
            return false;
        }

        const QString& altText = images[i].getAlt();
        ATProto::PostMaster::addImageToPost(post, std::move(blob), imgSize.width(), imgSize.height(), altText);
    }

    return true;
}

bool DraftPosts::addVideoToPost(ATProto::AppBskyFeed::Record::Post& post,
                                const VideoView& video,
                                const QString& draftsPath, const QString& baseName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    if (video.isNull())
        return true;

    auto blob = saveVideo(video.getPlaylistUrl(), video.getStartMs(), video.getEndMs(),
                          video.getRemoveAudio(), video.getNewHeight(), draftsPath, baseName);

    if (!blob)
    {
        dropVideo(draftsPath, baseName);
        return false;
    }

    const bool isGif = video.getPresentation() == QEnums::VIDEO_PRESENTATION_GIF;
    ATProto::PostMaster::addVideoToPost(post, std::move(blob), -1, -1, video.getAlt(), isGif);
    return true;
}

std::tuple<ATProto::Blob::SharedPtr, QSize> DraftPosts::saveImage(const QString& imgName,
                                               const QString& memeTopText, const QString& memeBottomText,
                                               const QString& draftsPath, const QString& baseName, int seq)
{
    qDebug() << "Save image:" << seq << imgName << "path:" << draftsPath << "base:" << baseName;
    QImage img = PhotoPicker::loadImage(imgName);

    if (img.isNull())
    {
        qWarning() << "Could not load image:" << seq << imgName;
        emit saveDraftPostFailed(tr("Could not load image #%1: %2").arg(seq + 1).arg(imgName));
        return { nullptr, QSize{} };
    }

    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = createAbsPath(draftsPath, imgFileName);
    qDebug() << "Draft image file name:" << fileName;

    if (!FileUtils::checkWriteMediaPermission())
    {
        qWarning() << "No permission to write media:" << fileName;
        emit saveDraftPostFailed(tr("Failed to save image #%1: %2").arg(seq + 1).arg(fileName));
        return { nullptr, QSize{} };
    }

    if (!img.save(fileName))
    {
        qWarning() << "Failed to save image:" << fileName;
        emit saveDraftPostFailed(tr("Failed to save image #%1: %2").arg(seq + 1).arg(fileName));
        return { nullptr, QSize{} };
    }

    auto blob = std::make_shared<ATProto::Blob>();
    blob->mRefLink = imgFileName;
    blob->mMimeType = "image/jpeg";
    blob->mSize = img.sizeInBytes();
    blob->mJson = blob->toJson();

    if (!memeTopText.isEmpty())
        blob->mJson.insert(Lexicon::DRAFT_MEME_TOP_TEXT_FIELD, memeTopText);

    if (!memeBottomText.isEmpty())
        blob->mJson.insert(Lexicon::DRAFT_MEME_BOTTOM_TEXT_FIELD, memeBottomText);

    return { blob, img.size() };
}

ATProto::Blob::SharedPtr DraftPosts::saveVideo(
        const QString& videoName, int videoStartMs, int videoEndMs, bool videoRemoveAudio,
        int videoNewHeight, const QString& draftsPath, const QString& baseName)
{
    qDebug() << "Save video:" << videoName << "path:" << draftsPath << "base:" << baseName;

    if (!videoName.startsWith("file://"))
    {
        qWarning() << "Invalid video:" << videoName;
        emit saveDraftPostFailed(tr("Could not load video: %1").arg(videoName));
        return nullptr;
    }

    const QString videoFileName = videoName.sliced(7);
    const QString draftFileName = createDraftVideoFileName(baseName);
    const QString absDraftFileName = createAbsPath(draftsPath, draftFileName);
    qDebug() << "Draft image file name:" << absDraftFileName;

    if (!FileUtils::checkWriteMediaPermission())
    {
        qWarning() << "No permission to write media:" << absDraftFileName;
        emit saveDraftPostFailed(tr("Failed to save video: %1").arg(absDraftFileName));
        return nullptr;
    }

    QFile fromFile(videoFileName);

    if (!fromFile.open(QFile::ReadOnly))
    {
        qWarning() << "Cannot open file:" << videoFileName << fromFile.errorString();
        emit saveDraftPostFailed(tr("Could not load video: %1").arg(videoFileName));
        return nullptr;
    }

    QFile toFile(absDraftFileName);

    if (!toFile.open(QFile::WriteOnly))
    {
        qWarning() << "Cannot create file:" << absDraftFileName << toFile.errorString();
        emit saveDraftPostFailed(tr("Failed to save video: %1").arg(absDraftFileName));
        return nullptr;
    }

    if (toFile.write(fromFile.readAll()) < 0)
    {
        qWarning() << "Failed to save video:" << absDraftFileName << toFile.errorString();
        toFile.close();
        toFile.remove();
        emit saveDraftPostFailed(tr("Failed to save video: %1").arg(absDraftFileName));
        return nullptr;
    }

    auto blob = std::make_shared<ATProto::Blob>();
    blob->mRefLink = draftFileName;
    blob->mMimeType = "video/mp4";

    if (videoStartMs > 0)
        blob->mJson.insert(Lexicon::DRAFT_VIDEO_START_MS_FIELD, videoStartMs);

    if (videoEndMs > 0)
        blob->mJson.insert(Lexicon::DRAFT_VIDEO_END_MS_FIELD, videoEndMs);

    if (videoRemoveAudio)
        blob->mJson.insert(Lexicon::DRAFT_VIDEO_REMOVE_AUDIO_FIELD, videoRemoveAudio);

    if (videoNewHeight > 0)
        blob->mJson.insert(Lexicon::DRAFT_VIDEO_NEW_HEIGHT_FIELD, videoNewHeight);

    return blob;
}

void DraftPosts::dropImages(const QString& draftsPath, const QString& baseName, int count) const
{
    for (int j = 0; j < count; ++j)
        dropImage(draftsPath, baseName, j);
}

void DraftPosts::dropImage(const QString& draftsPath, const QString& baseName, int seq) const
{
    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = createAbsPath(draftsPath, imgFileName);
    qDebug() << "Drop draft image:" << fileName;
    QFile::remove(fileName);
}

void DraftPosts::dropVideo(const QString& draftsPath, const QString& baseName)
{
    const QString videoFileName = createDraftVideoFileName(baseName);
    const QString fileName = createAbsPath(draftsPath, videoFileName);
    qDebug() << "Drop draft video:" << fileName;
    QFile::remove(fileName);
}

void DraftPosts::dropDraftPostFiles(const QString& draftsPath, const QString& fileName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString baseName = getBaseNameFromPostFileName(fileName);

    if (baseName.isEmpty())
    {
        qWarning() << "Invalid file name:" << fileName;
        return;
    }

    dropDraftPostFilesByBaseName(draftsPath, fileName);
}

void DraftPosts::dropDraftPostFilesByBaseName(const QString& draftsPath, const QString& baseName)
{
    qDebug() << "Remove post files:" << draftsPath << "baseName:" << baseName;
    QDir dir(draftsPath);
    const QString filePattern = QString("*%1*").arg(baseName);
    const auto files = dir.entryList({filePattern});

    for (const auto& file : files)
    {
        if (dir.remove(file))
            qDebug() << "Removed draft file:" << file << "in dir:" << draftsPath;
        else
            qWarning() << "Failed to remove draft file:" << file << "in dir:" << draftsPath;
    }
}

void DraftPosts::dropDraftPost(const QString& fileName)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString draftsPath = getDraftsPath();
    if (!draftsPath.isEmpty())
        dropDraftPostFiles(draftsPath, fileName);

    const QString picsPath = getPictureDraftsPath();
    if (!picsPath.isEmpty())
        dropDraftPostFiles(picsPath, fileName);
}

void DraftPosts::loadBlueskyDrafts(const QString& cursor)
{
    Q_ASSERT(mStorageType == STORAGE_BLUESKY);

    if (!bskyClient())
        return;

    getDraftPostsModel();

    if (mDraftPostsModel->isGetFeedInProgress())
    {
        qDebug() << "Already in progress";
        return;
    }

    mDraftPostsModel->setGetFeedInProgress(true);

    bskyClient()->getDrafts({}, Utils::makeOptionalString(cursor),
        [this, presence=getPresence()](ATProto::AppBskyDraft::GetDraftsOutput::SharedPtr output){
            if (!presence)
                return;

            mDraftPostsModel->setGetFeedInProgress(false);
            std::vector<ATProto::AppBskyFeed::PostFeed> postThreads;

            for (const auto& draftView : output->mDrafts)
            {
                // NOTE: we store the draft ID as post URI
                auto postFeed = convertDraftToFeedViewPost(*draftView, draftView->mId);
                postThreads.push_back(std::move(postFeed));
            }

            mDraftPostsModel->setFeed(std::move(postThreads), output->mCursor.value_or(""));
            emit draftsChanged();
            emit loadDraftPostsOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to get drafts:" << error << "-" << msg;
            mDraftPostsModel->setGetFeedInProgress(false);
            emit loadDraftPostsFailed(msg);
        });
}

void DraftPosts::loadBlueskyDraftsNextPage()
{
    if (!mDraftPostsModel)
        return;

    const QString& cursor = mDraftPostsModel->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed";
        return;
    }

    loadBlueskyDrafts(cursor);
}

bool DraftPosts::saveBlueskyDraftPost(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread)
{
    Q_ASSERT(mStorageType == STORAGE_BLUESKY);
    qDebug() << "Save Bluesky draft";

    if (!bskyClient())
        return false;

    const QString dateTime = FileUtils::createDateTimeName(draftPost->indexedAt());
    auto draft = createBlueskyDraft(draftPost, draftThread, dateTime);

    if (!draft)
    {
        dropDraftMedia(draftPost, draftThread, dateTime);
        return false;
    }

    bskyClient()->createDraft(draft,
        [this, presence=getPresence()](ATProto::AppBskyDraft::CreateDraftOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Saved draft:" << output->mId;
            emit saveDraftPostOk();
        },
        [this, presence=getPresence(), draftPost, draftThread, dateTime](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to save draft:" << error << "-" << msg;
            dropDraftMedia(draftPost, draftThread, dateTime);
            emit saveDraftPostFailed(msg);
        });

    return true;
}

ATProto::AppBskyDraft::Draft::SharedPtr DraftPosts::createBlueskyDraft(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread, const QString& baseName)
{
    auto draft = std::make_shared<ATProto::AppBskyDraft::Draft>();
    draft->mDeviceId = mSkywalker->getUserSettings()->getDeviceId();
    const QString deviceName = FileUtils::getDeviceName();
    draft->mDeviceName = QString("%1 (%2)").arg(deviceName, Skywalker::APP_NAME);

    draft->mPosts.reserve(draftThread.size() + 1);
    const auto& post = createBlueskyDraftPost(draftPost, baseName, 0);

    if (!post)
        return nullptr;

    draft->mPosts.push_back(post);

    Draft::ReplyToPost::SharedPtr replyToPost = createReplyToPost(
        draftPost->replyToUri(), draftPost->replyToAuthor(),
        draftPost->replyToText(), draftPost->replyToDateTime());

    if (replyToPost)
        draft->mJson.insert(Lexicon::DRAFT_REPPLY_TO, replyToPost->toJson());

    for (int i = 0; i < draftThread.size(); ++i)
    {
        const auto* draftThreadPost = draftThread[i];
        const auto& threadPost = createBlueskyDraftPost(draftThreadPost, baseName, i + 1);

        if (!threadPost)
            return nullptr;

        draft->mPosts.push_back(threadPost);
    }

    if (!draftPost->language().isEmpty())
        draft->mLangs.push_back(draftPost->language());

    draft->mDisableEmbedding = draftPost->embeddingDisabled();
    draft->mThreadgateRules = createThreadgateRules(draftPost);

    return draft;
}

void DraftPosts::dropDraftMedia(const DraftPostData* draftPost, const QList<DraftPostData*>& draftThread, const QString& baseName)
{
    const QString picDraftsPath = getPictureDraftsPath();

    if (picDraftsPath.isEmpty())
        return;

    dropImages(picDraftsPath, createPicBaseName(baseName, 0), draftPost->images().size());
    dropVideo(picDraftsPath, createPicBaseName(baseName, 0));

    for (int i = 0; i < draftThread.size(); ++i)
    {
        const auto* draftThreadPost = draftThread[i];
        dropImages(picDraftsPath, createPicBaseName(baseName, i + 1), draftThreadPost->images().size());
        dropVideo(picDraftsPath, createPicBaseName(baseName, i + 1));
    }
}

ATProto::AppBskyDraft::DraftPost::SharedPtr DraftPosts::createBlueskyDraftPost(const DraftPostData* draftPost, const QString& baseName, int threadIndex)
{
    auto post = std::make_shared<ATProto::AppBskyDraft::DraftPost>();
    post->mText = draftPost->text();
    post->mLabels = createSelfLabels(draftPost);

    ATProto::AppBskyFeed::PostReplyRef::SharedPtr replyRef = ATProto::PostMaster::createReplyRef(
        draftPost->replyToUri(), draftPost->replyToCid(),
        draftPost->replyRootUri(), draftPost->replyRootCid());

    if (replyRef)
        post->mJson.insert(Lexicon::DRAFT_REPPLY_REF, replyRef->toJson());

    if (!draftPost->images().empty())
    {
        const auto imageBaseName = createPicBaseName(baseName, threadIndex);
        post->mEmbedImages = createDraftEmbedImages(draftPost, imageBaseName);

        if (post->mEmbedImages.empty())
            return {};
    }

    if (!draftPost->video().isNull())
    {
        const auto videoBaseName = createPicBaseName(baseName, threadIndex);
        post->mEmbedVideos = createDraftEmbedVideos(draftPost, videoBaseName);

        if (post->mEmbedVideos.empty())
            return {};
    }

    post->mEmbedExternals = createDraftEmbedExternals(draftPost);
    post->mEmbedRecords = createDraftEmbedRecords(draftPost);

    if (!draftPost->embeddedLinks().empty())
    {
        auto links = createEmbeddedLinks(draftPost->embeddedLinks());
        post->mJson.insert(Lexicon::DRAFT_EMBBEDED_LINKS_FIELD, links->toJson());
    }

    return post;
}

ATProto::AppBskyDraft::DraftEmbedImage::List DraftPosts::createDraftEmbedImages(const DraftPostData* draftPost, const QString& baseName)
{
    const QString draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
    {
        emit saveDraftPostFailed(tr("Cannot create picture drafts path"));
        return {};
    }

    ATProto::AppBskyDraft::DraftEmbedImage::List embedImageList;
    const auto images = draftPost->images();
    embedImageList.reserve(images.size());

    for (int i = 0; i < images.size(); ++i)
    {
        const QString& imgName = images[i].getFullSizeUrl();
        const QString& memeTopText = images[i].getMemeTopText();
        const QString& memeBottomText = images[i].getMemeBottomText();
        auto [blob, imgSize] = saveImage(imgName, memeTopText, memeBottomText, draftsPath, baseName, i);

        if (!blob)
        {
            dropImages(draftsPath, baseName, i);
            return {};
        }

        auto embedImage = std::make_shared<ATProto::AppBskyDraft::DraftEmbedImage>();
        const QString& altText = images[i].getAlt();

        if (!altText.isEmpty())
            embedImage->mAlt = altText;

        embedImage->mLocalRef = std::make_shared<ATProto::AppBskyDraft::DraftEmbedLocalRef>();
        embedImage->mLocalRef->mPath = blob->mRefLink;

        embedImage->mJson = embedImage->toJson();

        if (!memeTopText.isEmpty())
            embedImage->mJson.insert(Lexicon::DRAFT_MEME_TOP_TEXT_FIELD, memeTopText);

        if (!memeBottomText.isEmpty())
            embedImage->mJson.insert(Lexicon::DRAFT_MEME_BOTTOM_TEXT_FIELD, memeBottomText);

        embedImageList.push_back(embedImage);
    }

    return embedImageList;
}

ATProto::AppBskyDraft::DraftEmbedVideo::List DraftPosts::createDraftEmbedVideos(const DraftPostData* draftPost, const QString& baseName)
{
    const QString draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
    {
        emit saveDraftPostFailed(tr("Cannot create picture drafts path"));
        return {};
    }

    const auto video = draftPost->video();
    Q_ASSERT(!video.isNull());
    auto blob = saveVideo(video.getPlaylistUrl(), video.getStartMs(), video.getEndMs(),
                          video.getRemoveAudio(), video.getNewHeight(), draftsPath, baseName);

    if (!blob)
    {
        dropVideo(draftsPath, baseName);
        return {};
    }

    auto embedVideo = std::make_shared<ATProto::AppBskyDraft::DraftEmbedVideo>();
    const QString& altText = video.getAlt();

    if (!altText.isEmpty())
        embedVideo->mAlt = altText;

    embedVideo->mLocalRef = std::make_shared<ATProto::AppBskyDraft::DraftEmbedLocalRef>();
    embedVideo->mLocalRef->mPath = blob->mRefLink;

    embedVideo->mJson = embedVideo->toJson();

    if (video.getStartMs() > 0)
        embedVideo->mJson.insert(Lexicon::DRAFT_VIDEO_START_MS_FIELD, video.getStartMs());

    if (video.getEndMs() > 0)
        embedVideo->mJson.insert(Lexicon::DRAFT_VIDEO_END_MS_FIELD, video.getEndMs());

    if (video.getRemoveAudio())
        embedVideo->mJson.insert(Lexicon::DRAFT_VIDEO_REMOVE_AUDIO_FIELD, video.getRemoveAudio());

    if (video.getNewHeight())
        embedVideo->mJson.insert(Lexicon::DRAFT_VIDEO_NEW_HEIGHT_FIELD, video.getNewHeight());

    const auto presentation = video.getPresentation();

    if (presentation != QEnums::VIDEO_PRESENTATION_DEFAULT && presentation != QEnums::VIDEO_PRESENTATION_UNKNOWN)
    {
        const QString rawPresentation = ATProto::AppBskyEmbed::videoPresentationToString((ATProto::AppBskyEmbed::VideoPresentation)presentation, "");
        embedVideo->mJson.insert(Lexicon::DRAFT_VIDEO_PRESENTATION, rawPresentation);
    }

    return { embedVideo };
}

ATProto::AppBskyDraft::DraftEmbedExternal::List DraftPosts::createDraftEmbedExternals(const DraftPostData* draftPost)
{   
    if (!draftPost->gif().isNull())
    {
        auto embedExternal = std::make_shared<ATProto::AppBskyDraft::DraftEmbedExternal>();
        const QUrl gifUrl = getGifUrl(draftPost->gif());
        embedExternal->mUri = gifUrl.toString();
        return { embedExternal };
    }
    else if (!draftPost->externalLink().isEmpty())
    {
        auto embedExternal = std::make_shared<ATProto::AppBskyDraft::DraftEmbedExternal>();
        embedExternal->mUri = draftPost->externalLink();
        return { embedExternal };
    }

    return {};
}

ATProto::ComATProtoLabel::SelfLabels::SharedPtr DraftPosts::createSelfLabels(const DraftPostData* draftPost) const
{
    if (draftPost->labels().empty())
        return nullptr;

    auto selfLabels = std::make_shared<ATProto::ComATProtoLabel::SelfLabels>();

    for (const auto& label : draftPost->labels())
    {
        auto selfLabel = std::make_shared<ATProto::ComATProtoLabel::SelfLabel>();
        selfLabel->mVal = label;
        selfLabels->mValues.push_back(selfLabel);
    }

    return selfLabels;
}

ATProto::AppBskyDraft::DraftEmbedRecord::List DraftPosts::createDraftEmbedRecords(const DraftPostData* draftPost)
{
    auto embedRecord = std::make_shared<ATProto::AppBskyDraft::DraftEmbedRecord>();
    embedRecord->mRecord = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();

    if (!draftPost->quoteUri().isEmpty())
    {
        embedRecord->mRecord->mUri = draftPost->quoteUri();
        embedRecord->mRecord->mCid = draftPost->quoteCid();
        return { embedRecord };
    }

    const auto quoteFeed = draftPost->quoteFeed();

    if (!quoteFeed.isNull())
    {
        embedRecord->mRecord->mUri = quoteFeed.getUri();
        embedRecord->mRecord->mCid = quoteFeed.getCid();
        return { embedRecord };
    }

    const auto quoteList = draftPost->quoteList();

    if (!quoteList.isNull())
    {
        embedRecord->mRecord->mUri = quoteList.getUri();
        embedRecord->mRecord->mCid = quoteList.getCid();
        return { embedRecord };
    }

    return {};
}

ATProto::AppBskyFeed::ThreadgateRules DraftPosts::createThreadgateRules(const DraftPostData* draftPost) const
{
    ATProto::AppBskyFeed::ThreadgateRules rules;

    if (!draftPost->restrictReplies())
        return rules;

    rules.mAllowMention = draftPost->allowMention();
    rules.mAllowFollower = draftPost->allowFollower();
    rules.mAllowFollowing = draftPost->allowFollowing();

    for (const auto& list : draftPost->allowLists())
    {
        auto listRule = std::make_shared<ATProto::AppBskyFeed::ThreadgateListRule>();
        listRule->mList = list;
        rules.mAllowList.push_back(listRule);
    }

    rules.mAllowNobody = !rules.mAllowMention && !rules.mAllowFollower && !rules.mAllowFollowing && rules.mAllowList.empty();
    return rules;
}

void DraftPosts::deleteBlueskyDraft(const QString& draftId, int index)
{
    Q_ASSERT(mStorageType == STORAGE_BLUESKY);
    qDebug() << "Delete draft:" << draftId;

    if (!bskyClient())
        return;

    getDraftPostsModel();

    bskyClient()->deleteDraft(draftId,
        [this, presence=getPresence(), index]{
            if (presence)
            {
                deleteMediaFiles(index);
                mDraftPostsModel->deleteDraft(index);
            }
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to delete draft:" << error << "-" << msg;
            emit deleteDraftFailed(msg);
        });
}

void DraftPosts::deleteMediaFiles(int index)
{
    const std::vector<Post> thread = mDraftPostsModel->getThread(index);
    const QString baseName = getMediaBaseName(thread);

    if (baseName.isEmpty())
        return;

    const QString draftsPath = getPictureDraftsPath();

    if (draftsPath.isEmpty())
    {
        qWarning() << "No pictures drafts path";
        return;
    }

    dropDraftPostFilesByBaseName(draftsPath, baseName);
}

QString DraftPosts::getMediaBaseName(const std::vector<Post>& thread) const
{
    for (const auto& post : thread)
    {
        auto images = post.getImages();

        if (images.empty())
        {
            const RecordWithMediaView::Ptr& record = post.getRecordWithMediaView();

            if (record)
                images = record->getImages();
        }

        if (!images.empty())
        {
            const QString& url = images.front().getFullSizeUrl();
            const QString baseName = getBaseNameFromMediaFullFileUrl(url);

            if (!baseName.isEmpty())
                return baseName;
        }

        VideoView::Ptr video = post.getVideoView();

        if (!video)
        {
            const RecordWithMediaView::Ptr& record = post.getRecordWithMediaView();

            if (record)
                video = record->getVideoView();
        }

        if (video)
        {
            const QString& url = video->getPlaylistUrl();
            const QString baseName = getBaseNameFromMediaFullFileUrl(url);

            if (!baseName.isEmpty())
                return baseName;
        }
    }

    return {};
}

void DraftPosts::getPostExternal(const Post& post, int index)
{
    qDebug() << "Get post external:" << post.getUri() << "index:" << index;

    auto externalView = post.getExternalView();
    const QString linkUri = externalView ? externalView->getUri() : "";

    if (linkUri.isEmpty())
    {
        qWarning() << "External link missing:" << post.getUri() << "index:" << index;
        return;
    }

    auto linkCardReader = std::make_shared<LinkCardReader>(this);

    connect(linkCardReader.get(), &LinkCardReader::linkCard, this,
        [this, linkCardReader, post, index](LinkCard* card){
            auto externalView = post.getExternalView();
            auto external = externalView->getExternal();
            external->mTitle = card->getTitle();
            external->mDescription = card->getDescription();

            if (!card->getThumb().isEmpty())
                external->mThumb = card->getThumb();

            mDraftPostsModel->updatePostExternal(post, index);
        });

    linkCardReader->getLinkCard(linkUri);
}

void DraftPosts::getPostRecord(const Post& post, int index)
{
    qDebug() << "Get post record:" << post.getUri() << "index:" << index;

    if (!bskyClient())
        return;

    auto recordView = post.getRecordViewFromRecordOrRecordWithMedia();

    if (!recordView || recordView->isNull())
    {
        qWarning() << "Record view missing:" << post.getUri() << "index:" << index;
        return;
    }

    const QString& uri = recordView->getNotFoundUri();

    if (uri.isEmpty())
    {
        qWarning() << "Record URI missing:" << post.getUri() << "index:" << index;
        return;
    }

    const ATProto::ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Record URI not valid:" << post.getUri() << "index:" << index;
        return;
    }

    if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_POST)
        getPostRecordPost(post, index, uri);
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_GENERATOR)
        getPostRecordFeed(post, index, uri);
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_GRAPH_LIST)
        getPostRecordList(post, index, uri);
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_GRAPH_STARTERPACK)
        getPostRecordStarterPack(post, index, uri);
    else
        qWarning() << "Record uri not supported:" << uri;
}

void DraftPosts::updatePostRecord(const Post& post, int index, const ATProto::AppBskyEmbed::Record* record, Draft::Quote::SharedPtr quote) const
{
    auto feedViewPost = post.getFeedViewPost();
    Q_ASSERT(feedViewPost);

    if (!feedViewPost)
    {
        qWarning() << "Feed view post missing:" << post.getUri() << "index:" << index;
        return;
    }

    if (feedViewPost->mPost->mEmbed->mType == ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW)
    {
        auto embed = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(feedViewPost->mPost->mEmbed->mEmbed);
        embed->mRecord = createRecordView(record, quote);
    }
    else
    {
        feedViewPost->mPost->mEmbed = std::make_shared<ATProto::AppBskyEmbed::EmbedView>();
        feedViewPost->mPost->mEmbed->mEmbed = createRecordView(record, quote);
        feedViewPost->mPost->mEmbed->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
    }

    const Post newPost(feedViewPost);
    mDraftPostsModel->updatePostRecord(newPost, index);
}

void DraftPosts::failUpdatePostRecord(const Post& post, int index, const QString& error)
{
    if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
    {
        qDebug() << "Record has been deleted:" << post.getUri() << "index:" << index;
        return;
    }

    mDraftPostsModel->updatePostRecordFailed(post, index);
}

void DraftPosts::getPostRecordPost(const Post& post, int index, const QString& postUri)
{
    qDebug() << "Get post record:" << post.getUri() << "index:" << index << "post:" << postUri;

    bskyClient()->getPosts({ postUri },
        [this, presence=getPresence(), post, index](ATProto::AppBskyFeed::PostView::List posts){
            if (!presence)
                return;

            if (posts.empty())
                return;

            const auto& recordPost = posts.front();
            auto quotePost = std::make_shared<Draft::QuotePost>();
            quotePost->mAuthor = recordPost->mAuthor;

            if (recordPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
                quotePost->mText = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(recordPost->mRecord)->mText;
            else
                qWarning() << "Unsupported record type:" << (int)recordPost->mRecordType;

            quotePost->mDateTime = recordPost->mIndexedAt;
            auto quote = std::make_shared<Draft::Quote>();
            quote->mRecordType = Draft::Quote::RecordType::QUOTE_POST;
            quote->mRecord = quotePost;

            auto record = std::make_shared<ATProto::AppBskyEmbed::Record>();
            record->mRecord = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();
            record->mRecord->mUri = recordPost->mUri;
            record->mRecord->mCid = recordPost->mCid;

            updatePostRecord(post, index, record.get(), quote);
        },
        [this, presence=getPresence(), post, index](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to get post record:" << error << "-" << msg;
            failUpdatePostRecord(post, index ,error);
        });
}

void DraftPosts::getPostRecordFeed(const Post& post, int index, const QString& feedUri)
{
    qDebug() << "Get post record:" << post.getUri() << "index:" << index << "feed:" << feedUri;

    bskyClient()->getFeedGenerator(feedUri,
        [this, presence=getPresence(), post, index](ATProto::AppBskyFeed::GetFeedGeneratorOutput::SharedPtr output){
            if (!presence)
                return;

            auto quote = std::make_shared<Draft::Quote>();
            quote->mRecordType = Draft::Quote::RecordType::QUOTE_FEED;
            quote->mRecord = output->mView;

            updatePostRecord(post, index, nullptr, quote);
        },
        [this, presence=getPresence(), post, index](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to get feed record:" << error << "-" << msg;
            failUpdatePostRecord(post, index ,error);
        });
}

void DraftPosts::getPostRecordList(const Post& post, int index, const QString& listUri)
{
    qDebug() << "Get post record:" << post.getUri() << "index:" << index << "list:" << listUri;

    bskyClient()->getList(listUri, 1, {},
        [this, presence=getPresence(), post, index](ATProto::AppBskyGraph::GetListOutput::SharedPtr output){
            if (!presence)
                return;

            auto quote = std::make_shared<Draft::Quote>();
            quote->mRecordType = Draft::Quote::RecordType::QUOTE_LIST;
            quote->mRecord = output->mList;

            updatePostRecord(post, index, nullptr, quote);
        },
        [this, presence=getPresence(), post, index](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to get list record:" << error << "-" << msg;
            failUpdatePostRecord(post, index ,error);
        });
}

void DraftPosts::getPostRecordStarterPack(const Post& post, int index, const QString& starterPackUri)
{
    qDebug() << "Get post record:" << post.getUri() << "index:" << index << "starterPack:" << starterPackUri;

    bskyClient()->getStarterPack(starterPackUri,
        [this, presence=getPresence(), post, index](ATProto::AppBskyGraph::StarterPackView::SharedPtr output){
            if (!presence)
                return;

            auto starterPackViewBasic = ATProto::GraphMaster::createStarterPackViewBasic(output);
            auto quote = std::make_shared<Draft::Quote>();
            quote->mRecordType = Draft::Quote::RecordType::QUOTE_STARTER_PACK;
            quote->mRecord = starterPackViewBasic;

            updatePostRecord(post, index, nullptr, quote);
        },
        [this, presence=getPresence(), post, index](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to get list record:" << error << "-" << msg;
            failUpdatePostRecord(post, index ,error);
        });
}

QUrl DraftPosts::getGifUrl(const TenorGif& gif) const
{
    // Pack all gif properties into the URI.
    QUrlQuery query{
        { "imageUrl", gif.getImageUrl() },
        { "gifWidth", QString::number(gif.getSize().width()) },
        { "gifHeight", QString::number(gif.getSize().height()) },
        { "smallWidth", QString::number(gif.getSmallSize().width()) },
        { "smallHeight", QString::number(gif.getSmallSize().height()) },
        { "smallUrl", gif.getSmallUrl() }
    };
    QUrl gifUrl(gif.getUrl());
    gifUrl.setQuery(query);

    return gifUrl;
}

void DraftPosts::addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const
{
    const QUrl gifUrl = getGifUrl(gif);
    ATProto::PostMaster::addExternalToPost(post, gifUrl.toString(), gif.getDescription(), "", nullptr);
}

void DraftPosts::addExternalLinkToPost(ATProto::AppBskyFeed::Record::Post& post, const QString& externalLink) const
{
    Q_ASSERT(!externalLink.isEmpty());
    ATProto::PostMaster::addExternalToPost(post, externalLink, "", "", nullptr);
}

}
