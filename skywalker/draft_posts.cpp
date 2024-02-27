// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "atproto_image_provider.h"
#include "definitions.h"
#include "content_filter.h"
#include "gif_utils.h"
#include "photo_picker.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

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

bool DraftPosts::hasDrafts() const
{
    return mDraftPostsModel && mDraftPostsModel->rowCount() > 0;
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

    ATProto::AppBskyFeed::PostReplyRef::Ptr replyRef = replyToUri.isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

    auto draft = std::make_shared<Draft>();
    draft->mPost = ATProto::PostMaster::createPostWithoutFacets(text, std::move(replyRef));
    ATProto::PostMaster::addLabelsToPost(*draft->mPost, labels);

    if (!quoteUri.isEmpty())
        ATProto::PostMaster::addQuoteToPost(*draft->mPost, quoteUri, quoteCid);

    if (!gif.isNull())
        addGifToPost(*draft->mPost, gif);

    if (restrictReplies)
    {
        draft->mThreadgate = ATProto::PostMaster::createThreadgate(
            getDraftUri("draft"), allowMention, allowFollowing, allowLists);
    }

    draft->mReplyToPost = createReplyToPost(replyToUri, replyToAuthor, replyToText, replyToDateTime);
    draft->mQuote = createQuote(quoteUri, quoteAuthor, quoteText, quoteDateTime, quoteFeed, quoteList);

    addImagesToPost(*draft->mPost, imageFileNames, altTexts,
        [this, presence=getPresence(), draft]{
            if (!presence)
                return;

            writeRecord(*draft);
        });
}

void DraftPosts::loadDraftPosts()
{
    listRecords();
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
    const Post& post = mDraftPostsModel->getPost(index);
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

void DraftPosts::removeDraftPost(const QString& recordUri)
{
    qDebug() << "Remove draft post:" << recordUri;
    deleteRecord(recordUri);
}

QString DraftPosts::getDraftUri(const QString& ref) const
{
    const QString userDid = mSkywalker->getUserDid();
    ATProto::ATUri atUri(userDid, COLLECTION_DRAFT_POST, ref);
    return atUri.toString();
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
    json.insert("$type", DRAFT_DEFS_QUOTE_POST);
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
        { DRAFT_DEFS_QUOTE_POST, RecordType::QUOTE_POST },
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

ATProto::AppBskyFeed::FeedViewPost::Ptr DraftPosts::convertDraftToFeedViewPost(Draft& draft, const QString& recordUri)
{
    auto feedView = std::make_unique<ATProto::AppBskyFeed::FeedViewPost>();
    feedView->mReply = createReplyRef(draft);
    feedView->mPost = convertDraftToPostView(draft, recordUri);
    return feedView;
}

ATProto::AppBskyFeed::PostView::Ptr DraftPosts::convertDraftToPostView(Draft& draft, const QString& recordUri)
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

ATProto::AppBskyFeed::ThreadgateView::Ptr DraftPosts::createThreadgateView(Draft& draft) const
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
    const ATProto::AppBskyEmbed::Embed* embed, Quote::Ptr quote)
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

    auto* imgProvider = ATProtoImageProvider::getProvider(ATProtoImageProvider::DRAFT_IMAGE);
    const QString host = bskyClient()->getHost();
    const QString did = mSkywalker->getUserDid();
    auto view = std::make_unique<ATProto::AppBskyEmbed::ImagesView>();

    for (const auto& image : images->mImages)
    {
        const QString& cid = image->mImage->mRefLink;
        const QString imgSource = imgProvider->createImageSource(host, did, cid);

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
    const ATProto::AppBskyEmbed::RecordWithMedia* record, Quote::Ptr quote)
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

QJsonObject DraftPosts::Draft::toJson() const
{
    QJsonObject json;
    json.insert("$type", COLLECTION_DRAFT_POST);
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

bool DraftPosts::writeRecord(const Draft& draft)
{
    if (!bskyClient())
        return false;

    const QString& repo = mSkywalker->getUserDid();
    const auto json = draft.toJson();

    bskyClient()->createRecord(repo, COLLECTION_DRAFT_POST, {}, json, false,
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
    if (!bskyClient())
        return;

    if (!mDraftPostsModel)
        mDraftPostsModel = mSkywalker->createDraftPostsModel();

    const QString& repo = mSkywalker->getUserDid();

    bskyClient()->listRecords(repo, COLLECTION_DRAFT_POST, 100, {},
        [this, presence=getPresence()](auto output) {
            if (!presence)
                return;

            ATProto::AppBskyFeed::PostFeed feed;

            for (const auto& record : output->mRecords)
            {
                try {
                    auto draft = Draft::fromJson(record->mValue);
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

QString DraftPosts::getImgSource(const QString& cid) const
{
    auto it = mCidImgSourceMap.find(cid);
    return it != mCidImgSourceMap.end() ? it->second : "";
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
                     const QStringList& imageFileNames, const QStringList& altTexts,
                     const std::function<void()>& continueCb, int imgSeq)
{
    if (imageFileNames.isEmpty())
    {
        continueCb();
        return;
    }

    emit uploadingImage(imgSeq);

    const QString imgName = imageFileNames.first();
    const auto remainingImageFileNames = imageFileNames.mid(1);
    const QString altText = altTexts.first();
    const auto remainingAltTexts = altTexts.mid(1);

    uploadImage(imgName,
        [this, presence=getPresence(), &post, altText, remainingImageFileNames, remainingAltTexts, continueCb, imgSeq](auto blob){
            if (!presence)
                return;

            ATProto::PostMaster::addImageToPost(post, std::move(blob), altText);
            addImagesToPost(post, remainingImageFileNames, remainingAltTexts, continueCb, imgSeq + 1);
        },
        [this, presence=getPresence()](const QString&, const QString& msg){
            if (!presence)
                return;

            emit saveDraftPostFailed(tr("Image upload failed: %1").arg(msg));
        });
}

}
