// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "atproto_image_provider.h"
#include "content_filter.h"
#include "file_utils.h"
#include "gif_utils.h"
#include "photo_picker.h"
#include "skywalker.h"
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
    return mDraftPostsModel->rowCount() < MAX_DRAFTS;
}

DraftPostData* DraftPosts::createDraft(const QString& text,
                                       const QStringList& imageFileNames, const QStringList& altTexts,
                                       const QString& replyToUri, const QString& replyToCid,
                                       const QString& replyRootUri, const QString& replyRootCid,
                                       const BasicProfile& replyToAuthor, const QString& replyToText,
                                       const QDateTime& replyToDateTime,
                                       const QString& quoteUri, const QString& quoteCid,
                                       const BasicProfile& quoteAuthor, const QString& quoteText,
                                       const QDateTime& quoteDateTime,
                                       const GeneratorView& quoteFeed, const ListView& quoteList,
                                       const TenorGif gif, const QStringList& labels,
                                       bool restrictReplies, bool allowMention, bool allowFollowing,
                                       const QStringList& allowLists,
                                       QDateTime timestamp)
{
    auto* draft = new DraftPostData(this);
    draft->setText(text);

    QList<ImageView> images;
    Q_ASSERT(imageFileNames.size() == altTexts.size());

    for (int i = 0; i < imageFileNames.size(); ++i)
    {
        ImageView view(imageFileNames[i], altTexts[i]);
        images.push_back(view);
    }

    draft->setImages(images);
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
    draft->setQuoteFeed(quoteFeed);
    draft->setQuoteList(quoteList);
    draft->setGif(gif);
    draft->setLabels(labels);
    draft->setRestrictReplies(restrictReplies);
    draft->setAllowMention(allowMention);
    draft->setAllowFollowing(allowFollowing);
    draft->setAllowLists(allowLists);

    return draft;
}

bool DraftPosts::saveDraftPost(const DraftPostData* draftPost)
{
    qDebug() << "Save draft post:" << draftPost->text();

    QString draftsPath;
    QString picDraftsPath;

    if (mStorageType == STORAGE_FILE)
    {
        draftsPath = getDraftsPath();

        if (draftsPath.isEmpty())
        {
            emit saveDraftPostFailed(tr("Cannot create app data path"));
            return false;
        }

        if (!draftPost->images().isEmpty())
        {
            picDraftsPath = getPictureDraftsPath();

            if (picDraftsPath.isEmpty())
            {
                emit saveDraftPostFailed(tr("Cannot create picture drafts path"));
                return false;
            }
        }
    }

    const QString dateTime = FileUtils::createDateTimeName(draftPost->indexedAt());

    ATProto::AppBskyFeed::PostReplyRef::Ptr replyRef = draftPost->replyToUri().isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(draftPost->replyToUri(), draftPost->replyToCid(),
                                                draftPost->replyRootUri(), draftPost->replyRootCid());

    auto draft = std::make_shared<Draft::Draft>();
    draft->mPost = ATProto::PostMaster::createPostWithoutFacets(draftPost->text(), std::move(replyRef));
    draft->mPost->mCreatedAt = draftPost->indexedAt();
    ATProto::PostMaster::addLabelsToPost(*draft->mPost, draftPost->labels());

    if (!draftPost->quoteUri().isEmpty())
        ATProto::PostMaster::addQuoteToPost(*draft->mPost, draftPost->quoteUri(), draftPost->quoteCid());

    if (mStorageType == STORAGE_FILE && !addImagesToPost(*draft->mPost, draftPost->images(), picDraftsPath, dateTime))
        return false;

    if (!draftPost->gif().isNull())
        addGifToPost(*draft->mPost, draftPost->gif());

    if (draftPost->restrictReplies())
    {
        const QString fileName = mStorageType == STORAGE_FILE ? createDraftPostFileName(dateTime) : "draft";
        draft->mThreadgate = ATProto::PostMaster::createThreadgate(
            getDraftUri(fileName), draftPost->allowMention(), draftPost->allowFollowing(), draftPost->allowLists());
    }

    draft->mReplyToPost = createReplyToPost(draftPost->replyToUri(), draftPost->replyToAuthor(),
                                            draftPost->replyToText(), draftPost->replyToDateTime());
    draft->mQuote = createQuote(draftPost->quoteUri(), draftPost->quoteAuthor(), draftPost->quoteText(),
                                draftPost->quoteDateTime(), draftPost->quoteFeed(), draftPost->quoteList());

    switch(mStorageType)
    {
    case STORAGE_FILE:
        if (!save(*draft, draftsPath, dateTime))
        {
            dropImages(picDraftsPath, dateTime, draftPost->images().size());
            return false;
        }

        break;
    case STORAGE_REPO:
        addImagesToPost(*draft->mPost, draftPost->images(),
            [this, presence=getPresence(), draft]{
                if (!presence)
                    return;

                writeRecord(*draft);
            });
        break;
    };

    return true;
}

void DraftPosts::loadDraftPosts()
{
    switch(mStorageType)
    {
    case STORAGE_FILE:
        loadDraftFeed();
        break;
    case STORAGE_REPO:
        listRecords();
        break;
    }
}

DraftPostsModel* DraftPosts::getDraftPostsModel()
{
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
        data->setQuoteAuthor(recordView->getAuthor().nonVolatileCopy());
        data->setQuoteText(recordView->getText());
        data->setQuoteDateTime(recordView->getIndexedAt());
    }
    else if (recordView->getFeedAvailable())
    {
        data->setQuoteFeed(recordView->getFeed());
    }
    else  if (recordView->getListAvailable())
    {
        data->setQuoteList(recordView->getList());
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
            ImageView draftImg(imgSource, imgView.getAlt());
            draftImages.push_back(draftImg);
        }
    }

    data->setImages(draftImages);
}

static void setGif(DraftPostData* data, const ExternalView* externalView)
{
    GifUtils gifUtils;

    if (gifUtils.isTenorLink(externalView->getUri()))
    {
        // NOTE: in addGifToPost several gif properties are packed into the URI as query params
        const QUrl uri(externalView->getUri());
        const QUrlQuery query(uri.query());
        const QString smallUrl = query.queryItemValue("smallUrl");
        const int smallWidth = query.queryItemValue("smallWidth").toInt();
        const int smallHeight = query.queryItemValue("smallHeight").toInt();

        // NOTE: The id is set to empty. This will avoid registration in Tenor::registerShare
        TenorGif gif("", externalView->getTitle(), "", uri.toString(QUrl::RemoveQuery),
                     smallUrl, QSize(smallWidth, smallHeight),
                     externalView->getThumbUrl(), QSize(1, 1));
        data->setGif(gif);
    }
}

static void setLabels(DraftPostData* data, const Post& post)
{
    QStringList labelNames;
    const auto labels = ContentFilter::getContentLabels(post.getLabels());

    for (const auto& label : labels)
        labelNames.push_back(label.getText());

    data->setLabels(labelNames);
}

static void setReplyRestrictions(DraftPostData* data, const Post& post)
{
    const auto restriction = post.getReplyRestriction();

    if (restriction == QEnums::REPLY_RESTRICTION_NONE)
        return;

    data->setRestrictReplies(true);

    if (restriction & QEnums::REPLY_RESTRICTION_NOBODY)
        return;

    data->setAllowMention(restriction & QEnums::REPLY_RESTRICTION_MENTIONED);
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

DraftPostData* DraftPosts::getDraftPostData(int index)
{
    if (index < 0 || index >= mDraftPostsModel->rowCount())
    {
        qWarning() << "Invalid index:" << index << "count:" << mDraftPostsModel->rowCount();
        return nullptr;
    }

    const Post& post = mDraftPostsModel->getPost(index);
    auto* data = new DraftPostData(this);
    data->setText(post.getText());
    setImages(data, post.getImages());
    data->setIndexedAt(post.getIndexedAt());
    data->setReplyToUri(post.getReplyToUri());
    data->setReplyToCid(post.getReplyToCid());
    data->setReplyRootUri(post.getReplyRootUri());
    data->setReplyRootCid(post.getReplyRootCid());

    if (post.getReplyToAuthor())
        data->setReplyToAuthor(post.getReplyToAuthor()->nonVolatileCopy());

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
        setRecordViewData(data, &recordMediaView->getRecord());
    }

    const auto externalView = post.getExternalView();
    if (externalView)
        setGif(data, externalView.get());

    setLabels(data, post);
    setReplyRestrictions(data, post);

    data->setRecordUri(post.getUri());

    return data;
}

void DraftPosts::removeDraftPost(int index)
{
    if (index < 0 || index >= mDraftPostsModel->rowCount())
    {
        qWarning() << "Invalid index:" << index << "count:" << mDraftPostsModel->rowCount();
        return;
    }

    const Post& post = mDraftPostsModel->getPost(index);

    // NOTE: with file storage, the record-uri is the file name;
    const QString& recordUri = post.getUri();
    qDebug() << "Remove draft post:" << index << "uri:" << recordUri;

    switch(mStorageType)
    {
    case STORAGE_FILE:
        dropDraftPost(recordUri);
        break;
    case STORAGE_REPO:
        deleteRecord(recordUri);
        break;
    }

    mDraftPostsModel->deleteDraft(index);
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
    const QString draftsPath = FileUtils::getPicturesPath(DRAFT_PICTURES_DIR);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << DRAFT_PICTURES_DIR;
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

ATProto::AppBskyActor::ProfileViewBasic::Ptr DraftPosts::createProfileViewBasic(const BasicProfile& author)
{
    if (author.isNull())
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyActor::ProfileViewBasic>();
    view->mDid = author.getDid();
    view->mHandle = author.getHandle();
    view->mDisplayName = author.getDisplayName();
    view->mAvatar = author.getAvatarUrl();

    return view;
}

ATProto::AppBskyActor::ProfileView::Ptr DraftPosts::createProfileView(const Profile& author)
{
    if (author.isNull())
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyActor::ProfileView>();
    view->mDid = author.getDid();
    view->mHandle = author.getHandle();
    view->mDisplayName = author.getDisplayName();
    view->mAvatar = author.getAvatarUrl();

    return view;
}

Draft::ReplyToPost::Ptr DraftPosts::createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                               const QString& text, const QDateTime& dateTime) const
{
    if (replyToUri.isEmpty())
        return nullptr;

    auto replyToPost = std::make_unique<Draft::ReplyToPost>();
    replyToPost->mAuthor = createProfileViewBasic(author);
    replyToPost->mText = text;
    replyToPost->mDateTime = dateTime;
    return replyToPost;
}

Draft::Quote::Ptr DraftPosts::createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
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

    auto quote = std::make_unique<Draft::Quote>();

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

Draft::QuotePost::Ptr DraftPosts::createQuotePost(const BasicProfile& author,
                                           const QString& text, const QDateTime& dateTime) const
{
    auto quotePost = std::make_unique<Draft::QuotePost>();
    quotePost->mAuthor = createProfileViewBasic(author);
    quotePost->mText = text;
    quotePost->mDateTime = dateTime;
    return quotePost;
}

ATProto::AppBskyFeed::GeneratorView::Ptr DraftPosts::createQuoteFeed(const GeneratorView& feed) const
{
    auto view = std::make_unique<ATProto::AppBskyFeed::GeneratorView>();
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

ATProto::AppBskyGraph::ListView::Ptr DraftPosts::createQuoteList(const ListView& list) const
{
    auto view = std::make_unique<ATProto::AppBskyGraph::ListView>();
    view->mUri = list.getUri();
    view->mCid = list.getCid();
    view->mCreator = createProfileView(list.getCreator());
    view->mName = list.getName();
    view->mPurpose = ATProto::AppBskyGraph::ListPurpose(list.getPurpose());
    view->mDescription = list.getDescription();
    view->mAvatar = list.getAvatar();
    return view;
}

ATProto::AppBskyFeed::FeedViewPost::Ptr DraftPosts::convertDraftToFeedViewPost(Draft::Draft& draft, const QString& recordUri)
{
    auto feedView = std::make_unique<ATProto::AppBskyFeed::FeedViewPost>();
    feedView->mReply = createReplyRef(draft);
    feedView->mPost = convertDraftToPostView(draft, recordUri);
    return feedView;
}

ATProto::AppBskyFeed::PostView::Ptr DraftPosts::convertDraftToPostView(Draft::Draft& draft, const QString& recordUri)
{
    auto postView = std::make_unique<ATProto::AppBskyFeed::PostView>();
    postView->mUri = recordUri;
    postView->mAuthor = createProfileViewBasic(mSkywalker->getUser());
    postView->mIndexedAt = draft.mPost->mCreatedAt;
    postView->mEmbed = createEmbedView(draft.mPost->mEmbed.get(), std::move(draft.mQuote));
    postView->mLabels = createContentLabels(*draft.mPost, recordUri);
    postView->mRecord = std::move(draft.mPost);
    postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    postView->mThreadgate = createThreadgateView(draft);

    return postView;
}

ATProto::AppBskyFeed::ThreadgateView::Ptr DraftPosts::createThreadgateView(Draft::Draft& draft) const
{
    if (!draft.mThreadgate)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyFeed::ThreadgateView>();
    view->mRecord = std::move(draft.mThreadgate);

    for (const auto& list : view->mRecord->mAllowList)
    {
        auto listView = std::make_unique<ATProto::AppBskyGraph::ListViewBasic>();
        listView->mUri = list->mList;
        listView->mName = list->mList;
        view->mLists.push_back(std::move(listView));
    }

    return view;
}

ATProto::AppBskyFeed::Record::Post::Ptr DraftPosts::createReplyToPost(const Draft::Draft& draft) const
{
    if (!draft.mReplyToPost)
        return nullptr;

    auto post = std::make_unique<ATProto::AppBskyFeed::Record::Post>();
    post->mText = draft.mReplyToPost->mText;
    post->mCreatedAt = draft.mReplyToPost->mDateTime;
    return post;
}

ATProto::AppBskyFeed::PostView::Ptr DraftPosts::convertReplyToPostView(Draft::Draft& draft) const
{
    if (!draft.mReplyToPost)
        return nullptr;

    Q_ASSERT(draft.mPost->mReply);
    Q_ASSERT(draft.mPost->mReply->mParent);

    if (!draft.mPost->mReply || !draft.mPost->mReply->mParent)
    {
        qWarning() << "Reply parent information missing from draft for reply:" << draft.mReplyToPost->mText;
        return nullptr;
    }

    auto view = std::make_unique<ATProto::AppBskyFeed::PostView>();
    view->mUri = draft.mPost->mReply->mParent->mUri;
    view->mCid = draft.mPost->mReply->mParent->mCid;
    view->mAuthor = std::move(draft.mReplyToPost->mAuthor);
    view->mRecord = createReplyToPost(draft);
    view->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    view->mIndexedAt = draft.mReplyToPost->mDateTime;
    return view;
}

ATProto::AppBskyFeed::ReplyRef::Ptr DraftPosts::createReplyRef(Draft::Draft& draft) const
{
    if (!draft.mReplyToPost)
        return nullptr;

    Q_ASSERT(draft.mPost->mReply);
    Q_ASSERT(draft.mPost->mReply->mRoot);

    if (!draft.mPost->mReply || !draft.mPost->mReply->mRoot)
    {
        qWarning() << "Reply root information missing from draft for reply:" << draft.mReplyToPost->mText;
        return nullptr;
    }

    auto replyRef = std::make_unique<ATProto::AppBskyFeed::ReplyRef>();

    replyRef->mParent = std::make_unique<ATProto::AppBskyFeed::ReplyElement>();
    replyRef->mParent->mType = ATProto::AppBskyFeed::PostElementType::POST_VIEW;
    replyRef->mParent->mPost = convertReplyToPostView(draft);

    // We did not save the root post in the draft. Set it to NOT FOUND. The post composer
    // does not need it.
    replyRef->mRoot = std::make_unique<ATProto::AppBskyFeed::ReplyElement>();
    replyRef->mRoot->mType = ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST;
    auto notFound = std::make_unique<ATProto::AppBskyFeed::NotFoundPost>();
    notFound->mUri = draft.mPost->mReply->mRoot->mUri;
    replyRef->mRoot->mPost = std::move(notFound);

    return replyRef;
}

ATProto::ComATProtoLabel::LabelList DraftPosts::createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& recordUri) const
{
    if (!post.mLabels)
        return {};

    const QString userDid = mSkywalker->getUserDid();
    ATProto::ComATProtoLabel::LabelList labels;

    for (const auto& selfLabel : post.mLabels->mValues)
    {
        auto label = std::make_unique<ATProto::ComATProtoLabel::Label>();
        label->mVal = selfLabel->mVal;
        label->mSrc = userDid;
        label->mUri = recordUri;
        label->mCreatedAt = post.mCreatedAt;
        labels.push_back(std::move(label));
    }

    return labels;
}

ATProto::AppBskyEmbed::EmbedView::Ptr DraftPosts::createEmbedView(
    const ATProto::AppBskyEmbed::Embed* embed, Draft::Quote::Ptr quote)
{
    if (!embed)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::EmbedView>();

    switch (embed->mType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mEmbed = createImagesView(std::get<ATProto::AppBskyEmbed::Images::Ptr>(embed->mEmbed).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::EXTERNAL:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
        view->mEmbed = createExternalView(std::get<ATProto::AppBskyEmbed::External::Ptr>(embed->mEmbed).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::RECORD:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW;
        view->mEmbed = createRecordView(std::get<ATProto::AppBskyEmbed::Record::Ptr>(embed->mEmbed).get(), std::move(quote));
        break;
    case ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
        view->mEmbed = createRecordWithMediaView(std::get<ATProto::AppBskyEmbed::RecordWithMedia::Ptr>(embed->mEmbed).get(), std::move(quote));
        break;
    case ATProto::AppBskyEmbed::EmbedType::UNKNOWN:
        qWarning() << "Unknown embed type";
        break;
    }

    return view;
}

ATProto::AppBskyEmbed::ImagesView::Ptr DraftPosts::createImagesView(const ATProto::AppBskyEmbed::Images* images)
{
    if (!images || images->mImages.empty())
        return nullptr;

    QString draftsPath;

    if (mStorageType == STORAGE_FILE)
    {
        draftsPath = getPictureDraftsPath();

        if (draftsPath.isEmpty())
            return nullptr;
    }

    auto* imgProvider = ATProtoImageProvider::getProvider(ATProtoImageProvider::DRAFT_IMAGE);
    const QString host = bskyClient()->getHost();
    const QString did = mSkywalker->getUserDid();
    auto view = std::make_unique<ATProto::AppBskyEmbed::ImagesView>();

    for (const auto& image : images->mImages)
    {
        QString imgSource;

        switch (mStorageType)
        {
        case STORAGE_FILE:
        {
            const QString path = createAbsPath(draftsPath, image->mImage->mRefLink);
            imgSource = "file://" + path;
            break;
        }
        case STORAGE_REPO:
        {
            const QString& cid = image->mImage->mRefLink;
            imgSource = imgProvider->createImageSource(host, did, cid);
            break;
        }
        }

        auto imgView = std::make_unique<ATProto::AppBskyEmbed::ImagesViewImage>();
        imgView->mThumb = imgSource;
        imgView->mFullSize = imgSource;
        imgView->mAlt = image->mAlt;
        view->mImages.push_back(std::move(imgView));
    }

    return view;
}

ATProto::AppBskyEmbed::ExternalView::Ptr DraftPosts::createExternalView(
    const ATProto::AppBskyEmbed::External* external) const
{
    if (!external || !external->mExternal)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::ExternalView>();
    view->mExternal = std::make_unique<ATProto::AppBskyEmbed::ExternalViewExternal>();
    // NOTE: the small gif size and url are encoded as URL params to this uri.
    view->mExternal->mUri = external->mExternal->mUri;
    view->mExternal->mTitle = external->mExternal->mTitle;
    view->mExternal->mDescription = external->mExternal->mDescription;

    const QUrl url(external->mExternal->mUri);
    const QUrlQuery query(url.query());
    view->mExternal->mThumb = query.queryItemValue("imageUrl");

    return view;
}

ATProto::AppBskyEmbed::RecordView::Ptr DraftPosts::createRecordView(
    const ATProto::AppBskyEmbed::Record* record, Draft::Quote::Ptr quote) const
{
    if (!record || !quote)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::RecordView>();

    switch (quote->mRecordType)
    {
    case Draft::Quote::RecordType::QUOTE_POST:
    {
        auto& quotePost = std::get<Draft::QuotePost::Ptr>(quote->mRecord);
        auto post = std::make_unique<ATProto::AppBskyFeed::Record::Post>();
        post->mText = quotePost->mText;
        post->mCreatedAt = quotePost->mDateTime;

        auto viewRecord = std::make_unique<ATProto::AppBskyEmbed::RecordViewRecord>();
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
        view->mRecord = std::move(std::get<ATProto::AppBskyFeed::GeneratorView::Ptr>(quote->mRecord));
        break;
    case Draft::Quote::RecordType::QUOTE_LIST:
        view->mRecordType = ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW;
        view->mRecord = std::move(std::get<ATProto::AppBskyGraph::ListView::Ptr>(quote->mRecord));
        break;
    default:
        qWarning() << "Unknown record type" << (int)quote->mRecordType;
        break;
    }

    return view;
}

ATProto::AppBskyEmbed::RecordWithMediaView::Ptr DraftPosts::createRecordWithMediaView(
    const ATProto::AppBskyEmbed::RecordWithMedia* record, Draft::Quote::Ptr quote)
{
    if (!record || !quote)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record->mRecord.get(), std::move(quote));

    switch (record->mMediaType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mMedia = createImagesView(std::get<ATProto::AppBskyEmbed::Images::Ptr>(record->mMedia).get());
        break;
    case ATProto::AppBskyEmbed::EmbedType::EXTERNAL:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW;
        view->mMedia = createExternalView(std::get<ATProto::AppBskyEmbed::External::Ptr>(record->mMedia).get());
        break;
    default:
        qWarning() << "Invalid media type:" << (int)record->mMediaType;
        break;
    }

    return view;
}

void DraftPosts::loadDraftFeed()
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
        return;

    if (!mDraftPostsModel)
        mDraftPostsModel = mSkywalker->createDraftPostsModel();

    auto fileList = getDraftPostFiles(draftsPath);
    ATProto::AppBskyFeed::PostFeed postFeed;

    for (const auto& file : fileList)
    {
        auto draft = loadDraft(file, draftsPath);

        if (draft)
        {
            auto feedViewPost = convertDraftToFeedViewPost(*draft, getDraftUri(file));
            postFeed.push_back(std::move(feedViewPost));
        }
        else
        {
            dropDraftPostFiles(draftsPath, file);
            const QString picDraftsPath = getPictureDraftsPath();

            if (!picDraftsPath.isEmpty())
                dropDraftPostFiles(picDraftsPath, file);
        }
    }

    mDraftPostsModel->setFeed(std::move(postFeed));
    emit draftsChanged();
    emit loadDraftPostsOk();
}

QStringList DraftPosts::getDraftPostFiles(const QString& draftsPath) const
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    QDir dir(draftsPath);
    return dir.entryList({"SWP_*.json"}, QDir::Files, QDir::Time);
}

Draft::Draft::Ptr DraftPosts::loadDraft(const QString& fileName, const QString& draftsPath) const
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
        auto blob = saveImage(imgName, draftsPath, baseName, i);

        if (!blob)
        {
            dropImages(draftsPath, baseName, i);
            return false;
        }

        const QString& altText = images[i].getAlt();
        ATProto::PostMaster::addImageToPost(post, std::move(blob), altText);
    }

    return true;
}

ATProto::Blob::Ptr DraftPosts::saveImage(const QString& imgName, const QString& draftsPath,
                                         const QString& baseName, int seq)
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    qDebug() << "Save image:" << seq << imgName << "path:" << draftsPath << "base:" << baseName;
    QImage img = PhotoPicker::loadImage(imgName);

    if (img.isNull())
    {
        qWarning() << "Could not load image:" << seq << imgName;
        emit saveDraftPostFailed(tr("Could not load image #%1: %2").arg(seq + 1).arg(imgName));
        return nullptr;
    }

    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = createAbsPath(draftsPath, imgFileName);
    qDebug() << "Draft image file name:" << fileName;

    if (!FileUtils::checkWriteMediaPermission())
    {
        qWarning() << "No permission to write media:" << fileName;
        return nullptr;
    }

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

void DraftPosts::dropImages(const QString& draftsPath, const QString& baseName, int count) const
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    for (int j = 0; j < count; ++j)
        dropImage(draftsPath, baseName, j);
}

void DraftPosts::dropImage(const QString& draftsPath, const QString& baseName, int seq) const
{
    Q_ASSERT(mStorageType == STORAGE_FILE);
    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = createAbsPath(draftsPath, imgFileName);
    qDebug() << "Drop draft image:" << fileName;
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

bool DraftPosts::writeRecord(const Draft::Draft& draft)
{
    Q_ASSERT(mStorageType == STORAGE_REPO);

    if (!bskyClient())
        return false;

    const QString& repo = mSkywalker->getUserDid();
    const auto json = draft.toJson();

    bskyClient()->createRecord(repo, Lexicon::COLLECTION_DRAFT_POST, {}, json, false,
        [this, presence=getPresence()](auto /* strongRef */) {
            if (!presence)
                return;

            emit saveDraftPostOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to write draft post:" << error << "-" << msg;
            emit saveDraftPostFailed(tr("Could not save draft: %1").arg(msg));
        });

    return true;
}

void DraftPosts::listRecords()
{
    Q_ASSERT(mStorageType == STORAGE_REPO);

    if (!bskyClient())
        return;

    if (!mDraftPostsModel)
        mDraftPostsModel = mSkywalker->createDraftPostsModel();

    const QString& repo = mSkywalker->getUserDid();

    bskyClient()->listRecords(repo, Lexicon::COLLECTION_DRAFT_POST, 100, {},
        [this, presence=getPresence()](auto output) {
            if (!presence)
                return;

            ATProto::AppBskyFeed::PostFeed feed;

            for (const auto& record : output->mRecords)
            {
                try {
                    auto draft = Draft::Draft::fromJson(record->mValue);
                    auto post = convertDraftToFeedViewPost(*draft, record->mUri);
                    feed.push_back(std::move(post));
                }
                catch (ATProto::InvalidJsonException& e) {
                    qWarning() << "Record format error:" << record->mUri << e.msg();
                    qInfo() << record->mValue;
                    deleteRecord(record->mUri);
                }
            }

            Q_ASSERT(mDraftPostsModel);
            mDraftPostsModel->setFeed(std::move(feed));

            emit draftsChanged();
            emit loadDraftPostsOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to list records:" << error << "-" << msg;
            emit loadDraftPostsFailed(tr("Failed to get drafts: %1").arg(msg));
        });
}

void DraftPosts::deleteRecord(const QString& recordUri)
{
    Q_ASSERT(mStorageType == STORAGE_REPO);
    ATProto::ATUri atUri(recordUri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << recordUri;
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->deleteRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(),
        [atUri]{
            qDebug() << "Deleted record:" << atUri.toString();
        },
        [atUri](const QString& error, const QString& msg){
            qWarning() << "Failed to delete record:" << atUri.toString() << error << "-" << msg;
        });
}

bool DraftPosts::uploadImage(const QString& imageName, const UploadImageSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mStorageType == STORAGE_REPO);
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    QByteArray imgData;
    const QString mimeType = PhotoPicker::createBlob(imgData, imageName);

    if (imgData.isEmpty())
    {
        qWarning() << "Image blob could not be created:" << imageName;
        return false;
    }

    bskyClient()->uploadBlob(imgData, mimeType,
        [presence=getPresence(), successCb](auto blob){
            if (!presence)
                return;

            successCb(std::move(blob));
        },
        [presence=getPresence(), errorCb](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Upload image failed:" << error << " - " << msg;
            errorCb(error, msg);
        });

    return true;
}

void DraftPosts::addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const
{
    // Pack all gif properties into the URI.
    QUrlQuery query{
        { "imageUrl", gif.getImageUrl() },
        { "smallWidth", QString::number(gif.getSmallSize().width()) },
        { "smallHeight", QString::number(gif.getSmallSize().height()) },
        { "smallUrl", gif.getSmallUrl() }
    };
    QUrl gifUrl(gif.getUrl());
    gifUrl.setQuery(query);

    ATProto::PostMaster::addExternalToPost(post, gifUrl.toString(), gif.getDescription(), "", nullptr);
}

void DraftPosts::addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                     const QList<ImageView>& images,
                     const std::function<void()>& continueCb, int imgSeq)
{
    Q_ASSERT(mStorageType == STORAGE_REPO);

    if (images.isEmpty())
    {
        continueCb();
        return;
    }

    emit uploadingImage(imgSeq);

    const QString imgName = images.first().getFullSizeUrl();
    const QString altText = images.first().getAlt();
    const auto remainingImages = images.mid(1);

    uploadImage(imgName,
        [this, presence=getPresence(), &post, altText, remainingImages, continueCb, imgSeq](auto blob){
            if (!presence)
                return;

            ATProto::PostMaster::addImageToPost(post, std::move(blob), altText);
            addImagesToPost(post, remainingImages, continueCb, imgSeq + 1);
        },
        [this, presence=getPresence()](const QString&, const QString& msg){
            if (!presence)
                return;

            emit saveDraftPostFailed(tr("Image upload failed: %1").arg(msg));
        });
}

}
