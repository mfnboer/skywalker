// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts.h"
#include "file_utils.h"
#include "photo_picker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* DRAFT_POSTS_DIR = "sw-draft-posts";

}

DraftPosts::DraftPosts(QObject* parent) :
    QObject(parent)
{
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
                               const QStringList& labels)
{
    Q_ASSERT(imageFileNames.size() == altTexts.size());
    qDebug() << "Save draft post:" << text;
    const QString draftsPath = FileUtils::getAppDataPath(DRAFT_POSTS_DIR);

    if (draftsPath.isEmpty())
    {
        qWarning() << "Failed to get path:" << DRAFT_POSTS_DIR;
        emit saveDraftPostFailed(tr("Cannot create app data path"));
        return;
    }

    const QString dateTime = FileUtils::createDateTimeName();

    ATProto::AppBskyFeed::PostReplyRef::Ptr replyRef = replyToUri.isEmpty() ? nullptr :
            ATProto::PostMaster::createReplyRef(replyToUri, replyToCid, replyRootUri, replyRootCid);

    auto post = ATProto::PostMaster::createPostWithoutFacets(text, std::move(replyRef));
    ATProto::PostMaster::addLabelsToPost(*post, labels);

    if (!quoteUri.isEmpty())
        ATProto::PostMaster::addQuoteToPost(*post, quoteUri, quoteCid);

    for (int i = 0; i < imageFileNames.size(); ++i)
    {
        const QString& imgName = imageFileNames[i];
        auto blob = saveImage(imgName, draftsPath, dateTime, i);

        if (!blob)
        {
            dropImages(draftsPath, dateTime, i);
            return;
        }

        const QString& altText = altTexts[i];
        ATProto::PostMaster::addImageToPost(*post, std::move(blob), altText);
    }

    // TODO threadgate

    auto replyToPost = createReplyToPost(replyToUri, replyToAuthor, replyToText, replyToDateTime);
    auto quote = createQuote(quoteUri, quoteAuthor, quoteText, quoteDateTime, quoteFeed, quoteList);

    if (!save(*post, std::move(replyToPost), std::move(quote), draftsPath, dateTime))
        dropImages(draftsPath, dateTime, imageFileNames.size());
}

QString DraftPosts::createDraftPostFileName(const QString& baseName) const
{
    return QString("SWP_%1.json").arg(baseName);
}

QString DraftPosts::createDraftImageFileName(const QString& baseName, int seq) const
{
    return QString("SWI%1_%2.jpg").arg(seq).arg(baseName);
}

ATProto::AppBskyActor::ProfileViewBasic::Ptr DraftPosts::createProfileViewBasic(const BasicProfile& author) const
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

ATProto::AppBskyActor::ProfileView::Ptr DraftPosts::createProfileView(const Profile& author) const
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
        quote->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
        quote->mRecord = createQuotePost(quoteAuthor, quoteText, quoteDateTime);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_FEED_GENERATOR)
    {
        quote->mRecordType = ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW;
        quote->mRecord = createQuoteFeed(quoteFeed);
    }
    else if (atUri.getCollection() == ATProto::ATUri::COLLECTION_GRAPH_LIST)
    {
        quote->mRecordType = ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW;
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

QJsonObject DraftPosts::QuotePost::toJson() const
{
    QJsonObject json;
    Q_ASSERT(mAuthor);

    if (mAuthor)
        json.insert("author", mAuthor->toJson());

    json.insert("text", mText);
    json.insert("date", mDateTime.toString(Qt::ISODateWithMs));
    return json;
}

QJsonObject DraftPosts::Quote::toJson() const
{
    QJsonObject json;
    json.insert("$type", ATProto::recordTypeToString(mRecordType));

    switch (mRecordType)
    {
    case ATProto::RecordType::APP_BSKY_FEED_POST:
        json.insert("post", std::get<QuotePost::Ptr>(mRecord)->toJson());
        break;
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
        json.insert("feed", std::get<ATProto::AppBskyFeed::GeneratorView::Ptr>(mRecord)->toJson());
        break;
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
        json.insert("list", std::get<ATProto::AppBskyGraph::ListView::Ptr>(mRecord)->toJson());
        break;
    default:
        qWarning() << "Unknown record type:" << (int)mRecordType;
        Q_ASSERT(false);
    }

    return json;
}

QJsonDocument DraftPosts::createJsonDoc(const ATProto::AppBskyFeed::Record::Post& post,
                                        ReplyToPost::Ptr replyToPost, Quote::Ptr quote) const
{
    QJsonObject jsonDraft;
    jsonDraft.insert("post", post.toJson());
    ATProto::XJsonObject::insertOptionalJsonObject<ReplyToPost>(jsonDraft, "replyToPost", replyToPost);
    ATProto::XJsonObject::insertOptionalJsonObject<Quote>(jsonDraft, "quote", quote);
    return QJsonDocument(jsonDraft);
}

bool DraftPosts::save(const ATProto::AppBskyFeed::Record::Post& post, ReplyToPost::Ptr replyToPost,
                      Quote::Ptr quote, const QString& draftsPath, const QString& baseName)
{
    const QString postFileName = createDraftPostFileName(baseName);
    const QString fileName = QString("%1/%2").arg(draftsPath, postFileName);
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

    const auto jsonDraft = createJsonDoc(post, std::move(replyToPost), std::move(quote));
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
    const QString fileName = QString("%1/%2").arg(draftsPath, imgFileName);
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

void DraftPosts::dropImages(const QString& draftsPath, const QString& baseName, int count)
{
    for (int j = 0; j < count; ++j)
        dropImage(draftsPath, baseName, j);
}

void DraftPosts::dropImage(const QString& draftsPath, const QString& baseName, int seq)
{
    const QString imgFileName = createDraftImageFileName(baseName, seq);
    const QString fileName = QString("%1/%2").arg(draftsPath, imgFileName);
    qDebug() << "Drop draft image:" << fileName;
    QFile::remove(fileName);
}

}
