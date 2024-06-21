// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"
#include "author_cache.h"
#include "user_settings.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

int Post::sNextGapId = 1;

Post Post::createGapPlaceHolder(const QString& gapCursor)
{
    Post post;
    post.mGapCursor = gapCursor;
    post.mGapId = sNextGapId++;
    return post;
}

Post Post::createNotFound()
{
    Post post;
    post.mNotFound = true;
    return post;
}

Post Post::createBlocked()
{
    Post post;
    post.mBlocked = true;
    return post;
}

Post Post::createNotSupported(const QString& unsupportedType)
{
    Post post;
    post.mNotSupported = true;
    post.mUnsupportedType = unsupportedType;
    return post;
}

Post Post::createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement)
{
    switch (threadElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST:
    {
        const auto threadPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(threadElement.mPost).get();
        Q_ASSERT(threadPost);
        Q_ASSERT(threadPost->mPost);
        return Post(threadPost->mPost.get(), -1);
    }
    case ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST:
        return Post::createNotFound();
    case ATProto::AppBskyFeed::PostElementType::BLOCKED_POST:
        return Post::createBlocked();
    case ATProto::AppBskyFeed::PostElementType::POST_VIEW:
    case ATProto::AppBskyFeed::PostElementType::UNKNOWN:
        return Post::createNotSupported(threadElement.mUnsupportedType);
    }

    Q_ASSERT(false);
    qWarning() << "Unexpected thread post type:" << int(threadElement.mType);
    return Post::createNotSupported(QString("Unexpected type: %1").arg(int(threadElement.mType)));
}

Post Post::createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement, int rawIndex)
{
    switch (replyElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::POST_VIEW:
    {
        const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(replyElement.mPost).get();
        Q_ASSERT(postView);
        return Post(postView, rawIndex);
    }
    case ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST:
        return Post::createNotFound();
    case ATProto::AppBskyFeed::PostElementType::BLOCKED_POST:
        return Post::createBlocked();
    case ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST:
    case ATProto::AppBskyFeed::PostElementType::UNKNOWN:
        return Post::createNotSupported(replyElement.mUnsupportedType);
    }

    Q_ASSERT(false);
    qWarning() << "Unexpected thread post type:" << int(replyElement.mType);
    return Post::createNotSupported(QString("Unexpected type: %1").arg(int(replyElement.mType)));
}

Post::Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost, int rawIndex) :
    mFeedViewPost(feedViewPost),
    mRawIndex(rawIndex)
{
    Q_ASSERT((feedViewPost && rawIndex >= 0) || (!feedViewPost && rawIndex == -1));

    if (feedViewPost)
    {
        mPost = feedViewPost->mPost.get();

        // Cache authors to minimize network requests for authors later.
        const BasicProfile profile = getAuthor();

        if (!profile.isNull())
            AuthorCache::instance().put(profile);

        if (feedViewPost->mReply)
        {
            if (feedViewPost->mReply->mGrandparentAuthor)
            {
                BasicProfile grandparent(feedViewPost->mReply->mGrandparentAuthor.get());
                AuthorCache::instance().put(grandparent);
            }
        }
    }
}

Post::Post(const ATProto::AppBskyFeed::PostView* postView, int rawIndex) :
    mPost(postView),
    mRawIndex(rawIndex)
{
    Q_ASSERT(postView);
    const BasicProfile profile = getAuthor();

    if (!profile.isNull())
        AuthorCache::instance().put(profile);
}

const QString& Post::getCid() const
{
    static const QString NO_STRING;
    return mPost ? mPost->mCid : NO_STRING;
}

const QString& Post::getUri() const
{
    static const QString NO_STRING;
    return mPost ? mPost->mUri : NO_STRING;
}

QString Post::getText() const
{
    static const QString NO_STRING;

    if (!mPost)
        return NO_STRING;

    if (mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

        if (record->mBridgyOriginalText && !record->mBridgyOriginalText->isEmpty())
            return UnicodeFonts::toPlainText(*record->mBridgyOriginalText);
        else
            return record->mText;
    }

    qWarning() << "Record type not supported:" << mPost->mRawRecordType;
    return NO_STRING;
}

QString Post::getFormattedText(const std::set<QString>& emphasizeHashtags) const
{
    static const QString NO_STRING;

    if (!mPost)
        return NO_STRING;

    if (mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

        if (record->mBridgyOriginalText && !record->mBridgyOriginalText->isEmpty())
            return *record->mBridgyOriginalText;

        return ATProto::RichTextMaster::getFormattedPostText(*record, UserSettings::getLinkColor(), emphasizeHashtags);
    }

    QString text = "UNSUPPORTED:\n" + mPost->mRawRecordType;
    return ATProto::RichTextMaster::plainToHtml(text);
}

BasicProfile Post::getAuthor() const
{
    return mPost ? BasicProfile(mPost->mAuthor.get()) : BasicProfile();
}

QDateTime Post::getIndexedAt() const
{
    if (!mPost)
        return {};

    // NOTE: the createdAt timestamp is not reliable as clients can put in a local timestamp
    // without timezone (seen in feeds)
    return mPost->mIndexedAt;
}

QDateTime Post::getTimelineTimestamp() const
{
    if (!mReplyRefTimestamp.isNull())
        return mReplyRefTimestamp;

    if (mFeedViewPost && mFeedViewPost->mReason)
        return mFeedViewPost->mReason->mIndexedAt;

    return getIndexedAt();
}

bool Post::isRepost() const
{
    return mFeedViewPost && mFeedViewPost->mReason;
}

std::optional<BasicProfile> Post::getRepostedBy() const
{
    if (!isRepost())
        return {};

    return BasicProfile(mFeedViewPost->mReason->mBy.get());
}

bool Post::isReply() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
        return true;

    if (!mPost)
        return false;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);
    return record->mReply.get();
}


std::optional<PostReplyRef> Post::getViewPostReplyRef() const
{
    if (!mFeedViewPost || !mFeedViewPost->mReply)
        return {};

    const auto& reply = *mFeedViewPost->mReply;
    PostReplyRef replyRef;
    replyRef.mRoot = Post::createPost(*reply.mRoot, mRawIndex);
    replyRef.mParent = Post::createPost(*reply.mParent, mRawIndex);

    // Set the reference timestamp to the timestap of this reply post.
    // They show up together with this reply post.
    replyRef.mRoot.mReplyRefTimestamp = getTimelineTimestamp();
    replyRef.mParent.mReplyRefTimestamp = getTimelineTimestamp();

    return replyRef;
}

std::optional<BasicProfile> Post::getReplyToAuthor() const
{
    if (mReplyToAuthor)
        return mReplyToAuthor;

    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        Q_ASSERT(mFeedViewPost->mReply->mParent);
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            return BasicProfile(postView->mAuthor.get());
        }
        else
        {
            return {};
        }
    }

    const auto did = getReplyToAuthorDid();
    if (did.isEmpty())
        return {};

    auto* author = AuthorCache::instance().get(did);
    if (!author)
        return {};

    const_cast<Post*>(this)->setReplyToAuthor(*author);
    return mReplyToAuthor;
}

ATProto::ComATProtoRepo::StrongRef::Ptr Post::getReplyToRef() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            auto ref = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
            ref->mCid = postView->mCid;
            ref->mUri = postView->mUri;
            return ref;
        }
    }

    if (!mPost)
        return nullptr;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return nullptr;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

    if (!record->mReply)
        return nullptr;

    return std::make_unique<ATProto::ComATProtoRepo::StrongRef>(*record->mReply->mParent);
}

QString Post::getReplyToCid() const
{
    const auto& ref = getReplyToRef();
    return ref ? ref->mCid : QString();
}

QString Post::getReplyToUri() const
{
    const auto& ref = getReplyToRef();
    return ref ? ref->mUri : QString();
}

QString Post::getReplyToAuthorDid() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            return postView->mAuthor->mDid;
        }
        else
        {
            return {};
        }
    }

    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

    if (!record->mReply)
        return {};

    const auto& uri = record->mReply->mParent->mUri;
    const ATProto::ATUri atUri(uri);

    if (!atUri.isValid())
        return {};

    if (atUri.authorityIsHandle())
        return {};

    const auto did = atUri.getAuthority();
    qDebug() << "Extracted did from uri:" << uri << "did:" << did;
    return did;
}

ATProto::ComATProtoRepo::StrongRef::Ptr Post::getReplyRootRef() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mRoot->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mRoot->mPost).get();
            auto ref = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
            ref->mCid = postView->mCid;
            ref->mUri = postView->mUri;
            return ref;
        }
    }

    if (!mPost)
        return nullptr;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return nullptr;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

    if (!record->mReply)
        return nullptr;

    return std::make_unique<ATProto::ComATProtoRepo::StrongRef>(*record->mReply->mRoot);
}

QString Post::getReplyRootCid() const
{
    const auto& ref = getReplyRootRef();
    return ref ? ref->mCid : QString();
}

QString Post::getReplyRootUri() const
{
    const auto& ref = getReplyRootRef();
    return ref ? ref->mUri : QString();
}

QList<ImageView> Post::getImages() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(mPost->mEmbed->mEmbed);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.push_back(ImageView(img.get()));

    return images;
}

ExternalView::Ptr Post::getExternalView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(mPost->mEmbed->mEmbed)->mExternal;
    return std::make_unique<ExternalView>(external.get());
}

RecordView::Ptr Post::getRecordView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordView::Ptr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordView>(*recordView);
}

RecordWithMediaView::Ptr Post::getRecordWithMediaView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::Ptr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordWithMediaView>(recordView.get());
}

bool Post::isQuotePost() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed)
        return {};

    return mPost->mEmbed->mType == ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW ||
           mPost->mEmbed->mType == ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW;
}

int Post::getReplyCount() const
{
    return mPost ? mPost->mReplyCount : 0;
}

int Post::getRepostCount() const
{
    return mPost ? mPost->mRepostCount : 0;
}

int Post::getLikeCount() const
{
    return mPost ? mPost->mLikeCount : 0;
}

QString Post::getRepostUri() const
{
    if (!mPost || !mPost->mViewer)
        return {};

    const auto& repost = mPost->mViewer->mRepost;
    return repost ? *repost : QString();
}

QString Post::getLikeUri() const
{
    if (!mPost || !mPost->mViewer)
        return {};

    const auto& like = mPost->mViewer->mLike;
    return like ? *like : QString();
}

bool Post::isReplyDisabled() const
{
    if (!mPost || !mPost->mViewer)
        return false;

    return mPost->mViewer->mReplyDisabled;
}

QEnums::ReplyRestriction Post::getReplyRestriction() const
{
    if (!mPost || !mPost->mThreadgate || !mPost->mThreadgate->mRecord)
        return QEnums::REPLY_RESTRICTION_NONE;

    const auto& threadgate = mPost->mThreadgate->mRecord;
    int restriction = QEnums::REPLY_RESTRICTION_NONE;

    if (threadgate->mAllowMention)
        restriction |= QEnums::REPLY_RESTRICTION_MENTIONED;

    if (threadgate->mAllowFollowing)
        restriction |= QEnums::REPLY_RESTRICTION_FOLLOWING;

    if (!threadgate->mAllowList.empty())
        restriction |= QEnums::REPLY_RESTRICTION_LIST;

    if (restriction != QEnums::REPLY_RESTRICTION_NONE)
        return (QEnums::ReplyRestriction)restriction;

    return QEnums::REPLY_RESTRICTION_NOBODY;
}

ListViewBasicList Post::getReplyRestrictionLists() const
{
    if (!mPost || !mPost->mThreadgate)
        return {};

    ListViewBasicList lists;

    for (const auto& l : mPost->mThreadgate->mLists)
    {
        ListViewBasic view(l.get());
        lists.append(view);
    }

    return lists;
}

const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& Post::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::Ptr> NO_LABELS;
    return mPost ? mPost->mLabels : NO_LABELS;
}

const LanguageList& Post::getLanguages() const
{
    if (!mLanguages.empty())
        return mLanguages;

    if (!mPost)
        return mLanguages;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return mLanguages;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);
    const_cast<Post*>(this)->mLanguages = LanguageUtils::getLanguages(record->mLanguages);
    return mLanguages;
}

bool Post::hasLanguage() const
{
    return !getLanguages().empty();
}

std::vector<QString> Post::getHashtags() const
{
    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);
    return ATProto::RichTextMaster::getFacetTags(*record);
}

}
