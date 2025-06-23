// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"
#include "post_utils.h"
#include "author_cache.h"
#include "content_filter.h"
#include "post_thread_cache.h"
#include "unicode_fonts.h"
#include "user_settings.h"
#include "lexicon/draft.h"
#include "lexicon/lexicon.h"
#include <atproto/lib/at_uri.h>
#include <atproto/lib/xjson.h>
#include <atproto/lib/rich_text_master.h>
#include <atproto/lib/xjson.h>

namespace Skywalker {

int Post::sNextGapId = 1;

Post Post::createGapPlaceHolder(const QString& gapCursor)
{
    Post post;
    post.mGapCursor = gapCursor;
    post.mGapId = sNextGapId++;
    return post;
}

Post Post::createHiddenPosts()
{
    Post post;
    post.mHiddenPosts = true;
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

Post Post::createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement, const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView)
{
    switch (threadElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST:
    {
        const auto threadPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(threadElement.mPost).get();
        Q_ASSERT(threadPost);
        Q_ASSERT(threadPost->mPost);
        Post post(threadPost->mPost);
        post.mThreadgateView = threadgateView;
        return post;
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

Post Post::createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement)
{
    switch (replyElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::POST_VIEW:
    {
        const auto postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(replyElement.mPost);
        Q_ASSERT(postView);
        return Post(postView);
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

Post::Post(const ATProto::AppBskyFeed::FeedViewPost::SharedPtr feedViewPost) :
    mFeedViewPost(feedViewPost)
{
    if (feedViewPost)
    {
        mPost = feedViewPost->mPost;

        // Cache authors to minimize network requests for authors later.
        const BasicProfile profile = getAuthor();

        if (!profile.isNull())
            AuthorCache::instance().put(profile);

        if (feedViewPost->mReply)
        {
            if (feedViewPost->mReply->mGrandparentAuthor)
            {
                BasicProfile grandparent(feedViewPost->mReply->mGrandparentAuthor);
                AuthorCache::instance().put(grandparent);
            }
        }
    }
}

Post::Post(const ATProto::AppBskyFeed::PostView::SharedPtr postView) :
    mPost(postView)
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
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

        if (record->mBridgyOriginalText && !record->mBridgyOriginalText->isEmpty())
            return UnicodeFonts::toPlainText(*record->mBridgyOriginalText);
        else
            return record->mText;
    }

    qWarning() << "Record type not supported:" << mPost->mRawRecordType;
    return NO_STRING;
}

QString Post::getFormattedText(const std::set<QString>& emphasizeHashtags, const QString& linkColor) const
{
    static const QString NO_STRING;

    if (!mPost)
        return NO_STRING;

    if (mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

        if (record->mBridgyOriginalText && !record->mBridgyOriginalText->isEmpty())
            return *record->mBridgyOriginalText;

        return ATProto::RichTextMaster::getFormattedPostText(
            *record,
            linkColor.isEmpty() ? UserSettings::getCurrentLinkColor() : linkColor,
            emphasizeHashtags);
    }

    QString text = "UNSUPPORTED:\n" + mPost->mRawRecordType;
    return ATProto::RichTextMaster::plainToHtml(text);
}

WebLink::List Post::getDraftEmbeddedLinks() const
{
    if (!mPost)
        return {};

    WebLink::List embeddedLinks;

    if (mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
        const ATProto::XJsonObject xjson(record->mJson);
        auto draftLinks = xjson.getOptionalObject<Draft::EmbeddedLinks>(Lexicon::DRAFT_EMBBEDED_LINKS_FIELD);

        if (draftLinks)
        {
            embeddedLinks.reserve(draftLinks->mEmbeddedLinks.size());

            for (const auto& link : draftLinks->mEmbeddedLinks)
                embeddedLinks.push_back(*link);
        }
    }

    return embeddedLinks;
}

BasicProfile Post::getAuthor() const
{
    return mPost ? BasicProfile(mPost->mAuthor) : BasicProfile();
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

    if (mFeedViewPost && mFeedViewPost->mReason && std::holds_alternative<ATProto::AppBskyFeed::ReasonRepost::SharedPtr>(*mFeedViewPost->mReason))
        return std::get<ATProto::AppBskyFeed::ReasonRepost::SharedPtr>(*mFeedViewPost->mReason)->mIndexedAt;

    return getIndexedAt();
}

bool Post::isRepost() const
{
    return mFeedViewPost && mFeedViewPost->mReason && std::holds_alternative<ATProto::AppBskyFeed::ReasonRepost::SharedPtr>(*mFeedViewPost->mReason);
}

std::optional<BasicProfile> Post::getRepostedBy() const
{
    if (!isRepost())
        return {};

    return BasicProfile(std::get<ATProto::AppBskyFeed::ReasonRepost::SharedPtr>(*mFeedViewPost->mReason)->mBy);
}

bool Post::isReply() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
        return true;

    if (!mPost)
        return false;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
    return record->mReply.get();
}


std::optional<PostReplyRef> Post::getViewPostReplyRef() const
{
    if (!mFeedViewPost || !mFeedViewPost->mReply)
        return {};

    const auto& reply = *mFeedViewPost->mReply;
    PostReplyRef replyRef;
    replyRef.mRoot = Post::createPost(*reply.mRoot);
    replyRef.mParent = Post::createPost(*reply.mParent);

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
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mParent->mPost).get();
            return BasicProfile(postView->mAuthor);
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

ATProto::ComATProtoRepo::StrongRef::SharedPtr Post::getReplyToRef() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto& postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mParent->mPost);
            auto ref = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();
            ref->mCid = postView->mCid;
            ref->mUri = postView->mUri;
            return ref;
        }
    }

    if (!mPost)
        return nullptr;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return nullptr;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

    if (!record->mReply)
        return nullptr;

    return std::make_shared<ATProto::ComATProtoRepo::StrongRef>(*record->mReply->mParent);
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
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mParent->mPost).get();
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

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

    if (!record->mReply)
        return {};

    const auto& uri = record->mReply->mParent->mUri;
    return PostUtils::extractDidFromUri(uri);
}

QString Post::getReplyRootAuthorDid() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mRoot->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mRoot->mPost).get();
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

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

    if (!record->mReply)
        return {};

    const auto& uri = record->mReply->mRoot->mUri;
    return PostUtils::extractDidFromUri(uri);
}

ATProto::ComATProtoRepo::StrongRef::SharedPtr Post::getReplyRootRef() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mRoot->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto& postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mRoot->mPost);
            auto ref = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();
            ref->mCid = postView->mCid;
            ref->mUri = postView->mUri;
            return ref;
        }
    }

    if (!mPost)
        return nullptr;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return nullptr;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);

    if (!record->mReply)
        return nullptr;

    return std::make_shared<ATProto::ComATProtoRepo::StrongRef>(*record->mReply->mRoot);
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

QString Post::getFeedContext() const
{
    if (!mFeedViewPost)
        return {};

    return mFeedViewPost->mFeedContext.value_or(QString{});
}

bool Post::hasUnknownEmbed() const
{
    if (!mPost || !mPost->mEmbed)
        return false;

    return mPost->mEmbed->mType == ATProto::AppBskyEmbed::EmbedViewType::UNKNOWN;
}

QString Post::getUnknownEmbedType() const
{
    if (!hasUnknownEmbed())
        return {};

    return mPost->mEmbed->mRawType;
}

QList<ImageView> Post::getImages() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mPost->mEmbed->mEmbed);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.push_back(ImageView(img));

    return images;
}

QList<ImageView> Post::getDraftImages() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mPost->mEmbed->mEmbed);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
    {
        ImageView view(img);
        const ATProto::XJsonObject xjson(img->mJson);
        const QString memeTopText = xjson.getOptionalString(Lexicon::DRAFT_MEME_TOP_TEXT_FIELD, "");
        const QString memeBottomText = xjson.getOptionalString(Lexicon::DRAFT_MEME_BOTTOM_TEXT_FIELD, "");
        view.setMemeTopText(memeTopText);
        view.setMemeBottomText(memeBottomText);
        images.push_back(view);
    }

    return images;
}

VideoView::Ptr Post::getVideoView() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW)
        return {};

    const auto& video = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(mPost->mEmbed->mEmbed);
    return std::make_unique<VideoView>(video);
}

VideoView::Ptr Post::getDraftVideoView() const
{
    auto videoView = getVideoView();

    if (!videoView)
        return {};

    const auto& atProtoView = videoView->getATProtoView();

    if (!atProtoView)
        return {};

    const ATProto::XJsonObject xjson(atProtoView->mJson);
    const int startMs = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_START_MS_FIELD, 0);
    const int endMs = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_END_MS_FIELD, 0);
    const bool removeAudio = xjson.getOptionalBool(Lexicon::DRAFT_VIDEO_REMOVE_AUDIO_FIELD, false);
    const int newHeight = xjson.getOptionalInt(Lexicon::DRAFT_VIDEO_NEW_HEIGHT_FIELD, 0);
    videoView->setStartMs(startMs);
    videoView->setEndMs(endMs);
    videoView->setRemoveAudio(removeAudio);
    videoView->setNewHeight(newHeight);

    return videoView;
}

ExternalView::Ptr Post::getExternalView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(mPost->mEmbed->mEmbed)->mExternal;
    return std::make_unique<ExternalView>(external);
}

RecordView::Ptr Post::getRecordView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::RECORD_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordView::SharedPtr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordView>(*recordView);
}

RecordWithMediaView::Ptr Post::getRecordWithMediaView() const
{
    if (!mPost)
        return {};
    
    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedViewType::RECORD_WITH_MEDIA_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordWithMediaView>(recordView);
}

RecordView::SharedPtr Post::getRecordViewFromRecordOrRecordWithMedia() const
{
    auto recordView = getRecordView();

    if (recordView)
        return recordView;

    auto recordWithMediaView = getRecordWithMediaView();

    if (!recordWithMediaView)
        return {};

    return recordWithMediaView->getRecordPtr();
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

int Post::getQuoteCount() const
{
    return mPost ? mPost->mQuoteCount : 0;
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

bool Post::isThreadMuted() const
{
    if (!mPost || !mPost->mViewer)
        return false;

    return mPost->mViewer->mThreadMuted;
}

bool Post::isReplyDisabled() const
{
    if (!mPost || !mPost->mViewer)
        return false;

    return mPost->mViewer->mReplyDisabled;
}

bool Post::isEmbeddingDisabled() const
{
    if (!mPost || !mPost->mViewer)
        return false;

    return mPost->mViewer->mEmbeddingDisabled;
}

bool Post::isViewerStatePinned() const
{
    if (!mPost || !mPost->mViewer)
        return false;

    return mPost->mViewer->mPinned;
}

ATProto::AppBskyFeed::ThreadgateView::SharedPtr Post::getThreadgateView() const
{
    if (mThreadgateView)
        return mThreadgateView;

    if (mPost && mPost->mThreadgate)
        return mPost->mThreadgate;

    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mRoot->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto& postView = std::get<ATProto::AppBskyFeed::PostView::SharedPtr>(mFeedViewPost->mReply->mRoot->mPost);

            if (postView->mThreadgate)
                return postView->mThreadgate;
        }
    }

    return nullptr;
}

QString Post::getThreadgateUri() const
{
    auto threadgate = getThreadgateView();

    if (threadgate && threadgate->mUri)
        return *threadgate->mUri;

    return {};
}

QEnums::ReplyRestriction Post::makeReplyRestriction(bool allowMention, bool allowFollower, bool allowFollowing, const bool allowList, bool allowNobody)
{
    if (allowNobody)
        return QEnums::REPLY_RESTRICTION_NOBODY;

    int restriction = QEnums::REPLY_RESTRICTION_NONE;

    if (allowMention)
        restriction |= QEnums::REPLY_RESTRICTION_MENTIONED;

    if (allowFollower)
        restriction |= QEnums::REPLY_RESTRICTION_FOLLOWER;

    if (allowFollowing)
        restriction |= QEnums::REPLY_RESTRICTION_FOLLOWING;

    if (allowList)
        restriction |= QEnums::REPLY_RESTRICTION_LIST;

    return (QEnums::ReplyRestriction)restriction;
}

QEnums::ReplyRestriction Post::getReplyRestriction() const
{
    auto threadgateView = getThreadgateView();

    if (!threadgateView || !threadgateView->mRecord)
        return QEnums::REPLY_RESTRICTION_NONE;

    const auto& threadgate = threadgateView->mRecord;
    return makeReplyRestriction(threadgate->mRules.mAllowMention,
                                threadgate->mRules.mAllowFollower,
                                threadgate->mRules.mAllowFollowing,
                                !threadgate->mRules.mAllowList.empty(),
                                threadgate->mRules.mAllowNobody);
}

ListViewBasicList Post::getReplyRestrictionLists() const
{
    auto threadgateView = getThreadgateView();

    if (!threadgateView)
        return {};

    ListViewBasicList lists;

    for (const auto& l : threadgateView->mLists)
    {
        ListViewBasic view(l->mUri, l->mCid, l->mName, l->mPurpose, l->mAvatar.value_or(""));
        lists.append(view);
    }

    return lists;
}

QStringList Post::getHiddenReplies() const
{
    auto threadgateView = getThreadgateView();

    if (!threadgateView || !threadgateView->mRecord)
        return {};

    QStringList hiddenReplies(threadgateView->mRecord->mHiddenReplies.begin(), threadgateView->mRecord->mHiddenReplies.end());
    return hiddenReplies;
}

bool Post::isHiddenReply() const
{
    auto threadgateView = getThreadgateView();

    if (!threadgateView || !threadgateView->mRecord)
        return false;

    return threadgateView->mRecord->mHiddenReplies.contains(getUri());
}

const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& Post::getLabels() const
{
    static const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr> NO_LABELS;
    return mPost ? mPost->mLabels : NO_LABELS;
}

const ContentLabelList& Post::getLabelsIncludingAuthorLabels() const
{
    if (mLabelsIncludingAuthorLabels)
        return *mLabelsIncludingAuthorLabels;

    const auto& author = getAuthor();
    ContentLabelList contentLabels = author.getContentLabels();
    ContentFilter::addContentLabels(contentLabels, getLabels());
    const_cast<Post*>(this)->mLabelsIncludingAuthorLabels = contentLabels;
    return *mLabelsIncludingAuthorLabels;
}

const LanguageList& Post::getLanguages() const
{
    if (!mLanguages.empty())
        return mLanguages;

    if (!mPost)
        return mLanguages;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return mLanguages;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
    const_cast<Post*>(this)->mLanguages = LanguageUtils::getLanguages(record->mLanguages);
    return mLanguages;
}

bool Post::hasLanguage() const
{
    return !getLanguages().empty();
}

QStringList Post::getMentionDids() const
{
    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
    return ATProto::RichTextMaster::getFacetMentionDids(*record);
}

std::vector<QString> Post::getHashtags() const
{
    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
    return ATProto::RichTextMaster::getFacetTags(*record);
}

std::vector<QString> Post::getWebLinks() const
{
    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mPost->mRecord);
    auto links = ATProto::RichTextMaster::getFacetLinks(*record);

    const auto externalView = getExternalView();

    if (externalView)
    {
        const auto uri = externalView->getUri();

        if (!uri.isEmpty())
            links.push_back(uri);
    }

    return links;
}

QEnums::TripleBool Post::isThread() const
{
    if (isPlaceHolder())
        return QEnums::TRIPLE_BOOL_NO;

    if (isReply())
        return QEnums::TRIPLE_BOOL_NO;

    if (getReplyCount() == 0)
        return QEnums::TRIPLE_BOOL_NO;

    const bool* isThread = PostThreadCache::instance().getIsThread(getUri());

    if (isThread == nullptr)
    {
        const QString text = getText();
        if (text.contains(UnicodeFonts::THREAD_SYMBOL))
        {
            // If the text contains the thread symbol we assume it is a thread.
            // Technically this may not be true, but for our use (display the symbol for
            // threads) it is good enough.
            PostThreadCache::instance().put(getUri(), true);
            return QEnums::TRIPLE_BOOL_YES;
        }

        return QEnums::TRIPLE_BOOL_UNKNOWN;
    }

    return *isThread ? QEnums::TRIPLE_BOOL_YES : QEnums::TRIPLE_BOOL_NO;
}

QJsonObject Post::toJson() const
{
    QJsonObject json;

    if (mFeedViewPost)
        json.insert("feedViewPost", mFeedViewPost->toJson());
    else if (mPost)
        json.insert("post", mPost->toJson());

    if (mGapId)
        json.insert("gapId", mGapId);
    if (!mGapCursor.isEmpty())
        json.insert("gapCursor", mGapCursor);
    if (mEndOfFeed)
        json.insert("endOfFeed", mEndOfFeed);
    if (mPostType != QEnums::POST_STANDALONE)
        json.insert("postType", QEnums::postTypeToString(mPostType));
    if (mFoldedPostType != QEnums::FOLDED_POST_NONE)
        json.insert("foldedPostType", QEnums::foldedPostTypeToString(mFoldedPostType));
    if (mThreadIndentLevel)
        json.insert("threadIndentLevel", mThreadIndentLevel);
    if (!mReplyRefTimestamp.isNull())
        json.insert("replyRefTimestamp", mReplyRefTimestamp.toString(Qt::ISODateWithMs));
    if (mReplyToAuthor && mReplyToAuthor->getProfileBasicView())
        json.insert("replyToAuthor", mReplyToAuthor->getProfileBasicView()->toJson());
    if (mParentInThread)
        json.insert("parentInThread", mParentInThread);
    if (mBlocked)
        json.insert("blocked", mBlocked);
    if (mNotFound)
        json.insert("notFound", mNotFound);
    if (mNotSupported)
        json.insert("notSupported", mNotSupported);
    if (!mUnsupportedType.isEmpty())
        json.insert("unsupportedType", mUnsupportedType);
    ATProto::XJsonObject::insertOptionalJsonObject<ATProto::AppBskyFeed::ThreadgateView>(json, "threadgateView", mThreadgateView);

    return json;
}

Post Post::fromJson(const QJsonObject& json)
{
    Post post;

    const ATProto::XJsonObject xjson(json);
    auto feedViewPost = xjson.getOptionalObject<ATProto::AppBskyFeed::FeedViewPost>("feedViewPost");

    if (feedViewPost)
    {
        post = Post(feedViewPost);
    }
    else
    {
        auto postView = xjson.getOptionalObject<ATProto::AppBskyFeed::PostView>("post");

        if (postView)
            post = Post(postView);
    }

    post.mGapId = xjson.getOptionalInt("gapId", 0);
    post.mGapCursor = xjson.getOptionalString("gapCursor", {});
    post.mEndOfFeed = xjson.getOptionalBool("endOfFeed", false);
    post.mPostType = QEnums::stringToPostType(xjson.getOptionalString("postType", "standalone"));
    post.mFoldedPostType = QEnums::stringToFoldedPostType(xjson.getOptionalString("foldedPostType", "none"));
    post.mThreadIndentLevel = xjson.getOptionalInt("threadIndentLevel", 0);
    post.mReplyRefTimestamp = xjson.getOptionalDateTime("replyRefTImestamp", {});
    auto profileBasicView = xjson.getOptionalObject<ATProto::AppBskyActor::ProfileViewBasic>("replyToAuthor");

    if (profileBasicView)
        post.mReplyToAuthor = BasicProfile(profileBasicView);

    post.mParentInThread = xjson.getOptionalBool("parentInThread", false);
    post.mBlocked = xjson.getOptionalBool("blocked", false);
    post.mNotFound = xjson.getOptionalBool("notFound", false);
    post.mNotSupported = xjson.getOptionalBool("notSupported", false);
    post.mUnsupportedType = xjson.getOptionalString("unsupportedType", {});
    post.mThreadgateView = xjson.getOptionalObject<ATProto::AppBskyFeed::ThreadgateView>("threadgateView");

    return post;
}

}
