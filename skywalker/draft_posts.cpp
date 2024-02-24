// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "author_cache.h"
#include "file_utils.h"
#include "gif_utils.h"
#include "photo_picker.h"
#include <atproto/lib/xjson.h>
#include <unordered_map>

namespace Skywalker {

namespace {

constexpr char const* DRAFT_POSTS_DIR = "sw-draft-posts";

QString createAbsPath(const QString& draftsPath, const QString& fileName)
{
    return QString("%1/%2").arg(draftsPath, fileName);
}

}

DraftPosts::DraftPosts(QObject* parent) :
    QObject(parent)
{
}

bool DraftPosts::hasDrafts() const
{
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
        return false;

    const auto files = getDraftPostFiles(draftsPath);
    return !files.empty();
}

void DraftPosts::saveDraftPost(const QString& text,
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
                               const QStringList& allowLists)
{
    Q_ASSERT(imageFileNames.size() == altTexts.size());
    qDebug() << "Save draft post:" << text;
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
    {
        emit saveDraftPostFailed(tr("Cannot create app data path"));
        return;
    }

    const QString dateTime = FileUtils::createDateTimeName();

    ATProto::AppBskyFeed::PostReplyRef::Ptr replyRef = replyToUri.isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

    auto draft = std::make_unique<Draft>();
    draft->mPost = ATProto::PostMaster::createPostWithoutFacets(text, std::move(replyRef));
    ATProto::PostMaster::addLabelsToPost(*draft->mPost, labels);

    if (!quoteUri.isEmpty())
        ATProto::PostMaster::addQuoteToPost(*draft->mPost, quoteUri, quoteCid);

    if (!addImagesToPost(*draft->mPost, imageFileNames, altTexts, draftsPath, dateTime))
        return;

    if (!gif.isNull())
        addGifToPost(*draft->mPost, gif);

    if (restrictReplies)
    {
        const QString fileName = createDraftPostFileName(dateTime);
        draft->mThreadgate = ATProto::PostMaster::createThreadgate(
                getDraftUri(fileName), allowMention, allowFollowing, allowLists);
    }

    draft->mReplyToPost = createReplyToPost(replyToUri, replyToAuthor, replyToText, replyToDateTime);
    draft->mQuote = createQuote(quoteUri, quoteAuthor, quoteText, quoteDateTime, quoteFeed, quoteList);

    if (!save(std::move(draft), draftsPath, dateTime))
        dropImages(draftsPath, dateTime, imageFileNames.size());
}

void DraftPosts::loadDraftPostsModel(DraftPostsModel* model)
{
    auto feed = loadDraftFeed();
    model->setFeed(std::move(feed));
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

DraftPostData* DraftPosts::getDraftPostData(const DraftPostsModel* model, int index)
{
    const Post& post = model->getPost(index);
    auto* data = new DraftPostData(this);
    data->setText(post.getText());
    setImages(data, post.getImages());
    data->setReplyToUri(post.getReplyToUri());
    data->setReplyToCid(post.getReplyToCid());
    data->setReplyRootUri(post.getReplyRootUri());
    data->setReplyRootCid(post.getReplyRootCid());

    if (post.getReplyToAuthor())
        data->setReplyToAuthor(*post.getReplyToAuthor());

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
        data->setImages(recordMediaView->getImages());
        setRecordViewData(data, &recordMediaView->getRecord());
    }

    const auto externalView = post.getExternalView();
    if (externalView)
    {
        GifUtils gifUtils;

        if (gifUtils.isTenorLink(externalView->getUri()))
        {
            const QUrl thumbUrl(externalView->getThumbUrl());
            const QUrlQuery query(thumbUrl.query());
            const QString smallUrl = query.queryItemValue("smallUrl");
            const int smallWidth = query.queryItemValue("smallWidth").toInt();
            const int smallHeight = query.queryItemValue("smallHeight").toInt();

            // NOTE: The id is set to empty. This will avoid restration in Tenor::registerShare
            TenorGif gif("", externalView->getTitle(), "", externalView->getUri(),
                         smallUrl, QSize(smallWidth, smallHeight),
                         thumbUrl.toString(QUrl::RemoveQuery), QSize(1, 1));
            data->setGif(gif);
        }
    }

    // TODO: labels
    // TODO: threadgate

    ATProto::ATUri atUri(post.getUri());
    if (atUri.isValid())
    {
        const QString fileName = atUri.getRkey();
        qDebug() << "Draf post file name:" << fileName;
        data->setDraftPostFileName(fileName);
    }
    else
    {
        qWarning() << "Invalid post at-uri:" << post.getUri();
    }

    return data;
}

void DraftPosts::removeDraftPost(const QString& fileName)
{
    qDebug() << "Remove draft post:" << fileName;
    dropDraftPost(getDraftsPath(), fileName);
}

QString DraftPosts::getDraftUri(const QString& fileName) const
{
    const QString userDid = AuthorCache::instance().getUser().getDid();
    ATProto::ATUri atUri(userDid, "skywalker.draft", fileName);
    return atUri.toString();
}

QString DraftPosts::getDraftsPath() const
{
    const auto did = AuthorCache::instance().getUser().getDid();
    const auto subDir = QString("%1/%2").arg(did, DRAFT_POSTS_DIR);
    const QString draftsPath = FileUtils::getAppDataPath(subDir);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << subDir;
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

DraftPosts::ReplyToPost::Ptr DraftPosts::createReplyToPost(const QString& replyToUri, const BasicProfile& author,
                                                           const QString& text, const QDateTime& dateTime) const
{
    if (replyToUri.isEmpty())
        return nullptr;

    auto replyToPost = std::make_unique<ReplyToPost>();
    replyToPost->mAuthor = createProfileViewBasic(author);
    replyToPost->mText = text;
    replyToPost->mDateTime = dateTime;
    return replyToPost;
}

DraftPosts::Quote::Ptr DraftPosts::createQuote(const QString& quoteUri, const BasicProfile& quoteAuthor,
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

    auto quote = std::make_unique<Quote>();

    if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_POST)
    {
        quote->mRecordType = Quote::RecordType::QUOTE_POST;
        quote->mRecord = createQuotePost(quoteAuthor, quoteText, quoteDateTime);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_GENERATOR)
    {
        quote->mRecordType = Quote::RecordType::QUOTE_FEED;
        quote->mRecord = createQuoteFeed(quoteFeed);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_GRAPH_LIST)
    {
        quote->mRecordType = Quote::RecordType::QUOTE_LIST;
        quote->mRecord = createQuoteList(quoteList);
    }
    else
    {
        qWarning() << "Unknown quote type:" << quoteUri;
        return nullptr;
    }

    return quote;
}

DraftPosts::QuotePost::Ptr DraftPosts::createQuotePost(const BasicProfile& author,
                                       const QString& text, const QDateTime& dateTime) const
{
    auto quotePost = std::make_unique<QuotePost>();
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

QJsonObject DraftPosts::ReplyToPost::toJson() const
{
    QJsonObject json;
    Q_ASSERT(mAuthor);

    if (mAuthor)
        json.insert("author", mAuthor->toJson());

    json.insert("text", mText);
    json.insert("date", mDateTime.toString(Qt::ISODateWithMs));
    return json;
}

DraftPosts::ReplyToPost::Ptr DraftPosts::ReplyToPost::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto replyToPost = std::make_unique<ReplyToPost>();
    replyToPost->mAuthor = xjson.getRequiredObject<ATProto::AppBskyActor::ProfileViewBasic>("author");
    replyToPost->mText = xjson.getRequiredString("text");
    replyToPost->mDateTime = xjson.getRequiredDateTime("date");
    return replyToPost;
}

QJsonObject DraftPosts::QuotePost::toJson() const
{
    QJsonObject json;
    json.insert("$type", "skywalker.draft.defs#quotePost");
    Q_ASSERT(mAuthor);

    if (mAuthor)
        json.insert("author", mAuthor->toJson());

    json.insert("text", mText);
    json.insert("date", mDateTime.toString(Qt::ISODateWithMs));
    return json;
}

DraftPosts::QuotePost::Ptr DraftPosts::QuotePost::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto quotePost = std::make_unique<QuotePost>();
    quotePost->mAuthor = xjson.getRequiredObject<ATProto::AppBskyActor::ProfileViewBasic>("author");
    quotePost->mText = xjson.getRequiredString("text");
    quotePost->mDateTime = xjson.getRequiredDateTime("date");
    return quotePost;
}

DraftPosts::Quote::RecordType DraftPosts::Quote::stringToRecordType(const QString& str)
{
    static const std::unordered_map<QString, RecordType> recordMapping = {
        { "skywalker.draft.defs#quotePost", RecordType::QUOTE_POST },
        { "app.bsky.feed.defs#generatorView", RecordType::QUOTE_FEED },
        { "app.bsky.graph.defs#listView", RecordType::QUOTE_LIST }
    };

    const auto it = recordMapping.find(str);
    if (it != recordMapping.end())
        return it->second;

    qWarning() << "Unknown record type:" << str;
    return RecordType::UNKNOWN;
}

QJsonObject DraftPosts::Quote::toJson() const
{
    QJsonObject json;

    switch (mRecordType)
    {
    case RecordType::QUOTE_POST:
        json.insert("record", std::get<QuotePost::Ptr>(mRecord)->toJson());
        break;
    case RecordType::QUOTE_FEED:
        json.insert("record", std::get<ATProto::AppBskyFeed::GeneratorView::Ptr>(mRecord)->toJson());
        break;
    case RecordType::QUOTE_LIST:
        json.insert("record", std::get<ATProto::AppBskyGraph::ListView::Ptr>(mRecord)->toJson());
        break;
    case RecordType::UNKNOWN:
        qWarning() << "Unknown record type:" << (int)mRecordType;
        Q_ASSERT(false);
        break;
    }

    return json;
}

DraftPosts::Quote::Ptr DraftPosts::Quote::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto quote = std::make_unique<Quote>();
    const auto recordJson = xjson.getRequiredJsonObject("record");
    const ATProto::XJsonObject recordXJson(recordJson);
    const QString rawRecordType = recordXJson.getRequiredString("$type");
    quote->mRecordType = stringToRecordType(rawRecordType);

    switch (quote->mRecordType)
    {
    case RecordType::QUOTE_POST:
        quote->mRecord = xjson.getRequiredObject<QuotePost>("record");
        break;
    case RecordType::QUOTE_FEED:
        quote->mRecord = xjson.getRequiredObject<ATProto::AppBskyFeed::GeneratorView>("record");
        break;
    case RecordType::QUOTE_LIST:
        quote->mRecord = xjson.getRequiredObject<ATProto::AppBskyGraph::ListView>("record");
        break;
    case RecordType::UNKNOWN:
        qWarning() << "Unknown record type:" << rawRecordType;
        throw ATProto::InvalidJsonException("Unknown record type");
    }

    return quote;
}

ATProto::AppBskyFeed::FeedViewPost::Ptr DraftPosts::convertDraftToFeedViewPost(Draft& draft, const QString& fileName, const QString& draftsPath) const
{
    auto feedView = std::make_unique<ATProto::AppBskyFeed::FeedViewPost>();
    feedView->mReply = createReplyRef(draft);
    feedView->mPost = convertDraftToPostView(draft, fileName, draftsPath);
    return feedView;
}

ATProto::AppBskyFeed::PostView::Ptr DraftPosts::convertDraftToPostView(Draft& draft, const QString& fileName, const QString& draftsPath) const
{
    auto postView = std::make_unique<ATProto::AppBskyFeed::PostView>();
    postView->mUri = getDraftUri(fileName);
    postView->mCid = "draft";
    postView->mAuthor = createProfileViewBasic(AuthorCache::instance().getUser());
    postView->mIndexedAt = draft.mPost->mCreatedAt;
    postView->mEmbed = createEmbedView(draft.mPost->mEmbed.get(), std::move(draft.mQuote), draftsPath);
    postView->mLabels = createContentLabels(*draft.mPost, fileName);
    postView->mRecord = std::move(draft.mPost);
    postView->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    postView->mThreadgate = createThreadgateView(draft);

    return postView;
}

ATProto::AppBskyFeed::ThreadgateView::Ptr DraftPosts::createThreadgateView(Draft& draft) const
{
    if (!draft.mThreadgate)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyFeed::ThreadgateView>();
    view->mRecord = std::move(draft.mThreadgate);
    return view;
}

ATProto::AppBskyFeed::Record::Post::Ptr DraftPosts::createReplyToPost(const Draft& draft) const
{
    if (!draft.mReplyToPost)
        return nullptr;

    auto post = std::make_unique<ATProto::AppBskyFeed::Record::Post>();
    post->mText = draft.mReplyToPost->mText;
    post->mCreatedAt = draft.mReplyToPost->mDateTime;
    return post;
}

ATProto::AppBskyFeed::PostView::Ptr DraftPosts::convertReplyToPostView(Draft& draft) const
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

ATProto::AppBskyFeed::ReplyRef::Ptr DraftPosts::createReplyRef(Draft& draft) const
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

ATProto::ComATProtoLabel::LabelList DraftPosts::createContentLabels(const ATProto::AppBskyFeed::Record::Post& post, const QString& fileName) const
{
    if (!post.mLabels)
        return {};

    const QString userDid = AuthorCache::instance().getUser().getDid();
    ATProto::ComATProtoLabel::LabelList labels;

    for (const auto& selfLabel : post.mLabels->mValues)
    {
        auto label = std::make_unique<ATProto::ComATProtoLabel::Label>();
        label->mVal = selfLabel->mVal;
        label->mSrc = userDid;
        label->mUri = getDraftUri(fileName);
        label->mCreatedAt = post.mCreatedAt;
    }

    return labels;
}

ATProto::AppBskyEmbed::EmbedView::Ptr DraftPosts::createEmbedView(
    const ATProto::AppBskyEmbed::Embed* embed, Quote::Ptr quote, const QString& draftsPath) const
{
    if (!embed)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::EmbedView>();

    switch (embed->mType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mEmbed = createImagesView(std::get<ATProto::AppBskyEmbed::Images::Ptr>(embed->mEmbed).get(), draftsPath);
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
        view->mEmbed = createRecordWithMediaView(std::get<ATProto::AppBskyEmbed::RecordWithMedia::Ptr>(embed->mEmbed).get(), std::move(quote), draftsPath);
        break;
    case ATProto::AppBskyEmbed::EmbedType::UNKNOWN:
        qWarning() << "Unknown embed type";
        break;
    }

    return view;
}

ATProto::AppBskyEmbed::ImagesView::Ptr DraftPosts::createImagesView(
    const ATProto::AppBskyEmbed::Images* images, const QString& draftsPath) const
{
    if (!images || images->mImages.empty())
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::ImagesView>();

    for (const auto& image : images->mImages)
    {
        auto imgView = std::make_unique<ATProto::AppBskyEmbed::ImagesViewImage>();
        const QString path = createAbsPath(draftsPath, image->mImage->mRefLink);
        imgView->mThumb = "file://" + path;
        imgView->mFullSize = "file://" + path;
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
    view->mExternal->mUri = external->mExternal->mUri;
    view->mExternal->mTitle = external->mExternal->mTitle;
    view->mExternal->mDescription = external->mExternal->mDescription;

    // NOTE: the small gif size and url are encoded as URL params to this thumbnail image.
    view->mExternal->mThumb = external->mExternal->mThumb->mRefLink;
    return view;
}

ATProto::AppBskyEmbed::RecordView::Ptr DraftPosts::createRecordView(
    const ATProto::AppBskyEmbed::Record* record, Quote::Ptr quote) const
{
    if (!record || !quote)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::RecordView>();

    switch (quote->mRecordType)
    {
    case Quote::RecordType::QUOTE_POST:
    {
        auto& quotePost = std::get<QuotePost::Ptr>(quote->mRecord);
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
    case Quote::RecordType::QUOTE_FEED:
        view->mRecordType = ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW;
        view->mRecord = std::move(std::get<ATProto::AppBskyFeed::GeneratorView::Ptr>(quote->mRecord));
        break;
    case Quote::RecordType::QUOTE_LIST:
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
    const ATProto::AppBskyEmbed::RecordWithMedia* record, Quote::Ptr quote, const QString& draftsPath) const
{
    if (!record || !quote)
        return nullptr;

    auto view = std::make_unique<ATProto::AppBskyEmbed::RecordWithMediaView>();
    view->mRecord = createRecordView(record->mRecord.get(), std::move(quote));

    switch (record->mMediaType)
    {
    case ATProto::AppBskyEmbed::EmbedType::IMAGES:
        view->mMediaType = ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW;
        view->mMedia = createImagesView(std::get<ATProto::AppBskyEmbed::Images::Ptr>(record->mMedia).get(), draftsPath);
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

QJsonObject DraftPosts::Draft::toJson() const
{
    QJsonObject json;
    json.insert("post", mPost->toJson());
    ATProto::XJsonObject::insertOptionalJsonObject<ReplyToPost>(json, "replyToPost", mReplyToPost);
    ATProto::XJsonObject::insertOptionalJsonObject<Quote>(json, "quote", mQuote);
    ATProto::XJsonObject::insertOptionalJsonObject<ATProto::AppBskyFeed::Threadgate>(json, "threadgate", mThreadgate);
    return json;
}

DraftPosts::Draft::Ptr DraftPosts::Draft::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto draft = std::make_unique<Draft>();
    draft->mPost = xjson.getRequiredObject<ATProto::AppBskyFeed::Record::Post>("post");
    draft->mReplyToPost = xjson.getOptionalObject<ReplyToPost>("replyToPost");
    draft->mQuote = xjson.getOptionalObject<Quote>("quote");
    draft->mThreadgate = xjson.getOptionalObject<ATProto::AppBskyFeed::Threadgate>("threadgate");
    return draft;
}

bool DraftPosts::save(Draft::Ptr draft, const QString& draftsPath, const QString& baseName)
{
    const QString postFileName = createDraftPostFileName(baseName);
    const QString fileName = createAbsPath(draftsPath, postFileName);
    qDebug() << "Draft post name:" << fileName;
    QFile file(fileName);

    if (file.exists())
    {
        qWarning() << "File already exists:" << fileName;
        emit saveDraftPostFailed(tr("File already exists: %1").arg(fileName));
        return false;
    }

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot create file:" << fileName;
        emit saveDraftPostFailed(tr("Cannot create file: %1").arg(fileName));
        return false;
    }

    const auto jsonDraft = QJsonDocument(draft->toJson());
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

ATProto::Blob::Ptr DraftPosts::saveImage(const QString& imgName, const QString& draftsPath,
                                         const QString& baseName, int seq)
{
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

void DraftPosts::addGifToPost(ATProto::AppBskyFeed::Record::Post& post, const TenorGif& gif) const
{
    QUrlQuery query{
        { "smallWidth", QString::number(gif.getSmallSize().width()) },
        { "smallHeight", QString::number(gif.getSmallSize().height()) },
        { "smallUrl", gif.getSmallUrl() }
    };
    QUrl imageUrl(gif.getImageUrl());
    imageUrl.setQuery(query);

    auto blob = std::make_unique<ATProto::Blob>();
    blob->mRefLink = imageUrl.toString();
    blob->mMimeType = "image";
    blob->mSize = 0;

    ATProto::PostMaster::addExternalToPost(post, gif.getUrl(), gif.getDescription(), "", std::move(blob));
}

bool DraftPosts::addImagesToPost(ATProto::AppBskyFeed::Record::Post& post,
                     const QStringList& imageFileNames, const QStringList& altTexts,
                     const QString& draftsPath, const QString& baseName)
{
    for (int i = 0; i < imageFileNames.size(); ++i)
    {
        const QString& imgName = imageFileNames[i];
        auto blob = saveImage(imgName, draftsPath, baseName, i);

        if (!blob)
        {
            dropImages(draftsPath, baseName, i);
            return false;
        }

        const QString& altText = altTexts[i];
        ATProto::PostMaster::addImageToPost(post, std::move(blob), altText);
    }

    return true;
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

void DraftPosts::dropDraftPost(const QString& draftsPath, const QString& fileName)
{
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

QStringList DraftPosts::getDraftPostFiles(const QString& draftsPath) const
{
    QDir dir(draftsPath);
    return dir.entryList({"SWP_*.json"}, QDir::Files, QDir::Time);
}

DraftPosts::Draft::Ptr DraftPosts::loadDraft(const QString& fileName, const QString& draftsPath) const
{
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

    try {
        auto draft = Draft::fromJson(doc.object());
        return draft;
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << "File format error:" << e.msg();
        qInfo() << doc.object();
        return nullptr;
    }
}

ATProto::AppBskyFeed::PostFeed DraftPosts::loadDraftFeed()
{
    const QString draftsPath = getDraftsPath();

    if (draftsPath.isEmpty())
        return {};

    auto fileList = getDraftPostFiles(draftsPath);
    ATProto::AppBskyFeed::PostFeed postFeed;

    for (const auto& file : fileList)
    {
        auto draft = loadDraft(file, draftsPath);

        if (draft)
        {
            auto feedViewPost = convertDraftToFeedViewPost(*draft, file, draftsPath);
            postFeed.push_back(std::move(feedViewPost));
        }
        else
        {
            dropDraftPost(draftsPath, file);
        }
    }

    return postFeed;
}

}
