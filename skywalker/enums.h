// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <atproto/lib/lexicon/chat_bsky_convo.h>
#include <atproto/lib/lexicon/app_bsky_graph.h>
#include <atproto/lib/lexicon/app_bsky_notification.h>
#include <atproto/lib/lexicon/com_atproto_moderation.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

// enums that can be used in QML
class QEnums : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum DisplayMode
    {
        DISPLAY_MODE_SYSTEM = 0,
        DISPLAY_MODE_LIGHT = 1,
        DISPLAY_MODE_DARK = 2
    };
    Q_ENUM(DisplayMode)

    enum PostType
    {
        POST_STANDALONE = 0,
        POST_ROOT,
        POST_REPLY,
        POST_LAST_REPLY,
        POST_THREAD // only in thread model
    };
    Q_ENUM(PostType)

    static QString postTypeToString(PostType postType);
    static PostType stringToPostType(const QString& str);

    // A sequence of thread posts can be folded up in the timeline.
    enum FoldedPostType
    {
        FOLDED_POST_NONE,
        FOLDED_POST_FIRST,
        FOLDED_POST_SUBSEQUENT
    };
    Q_ENUM(FoldedPostType)

    static QString foldedPostTypeToString(FoldedPostType foldedPostType);
    static FoldedPostType stringToFoldedPostType(const QString& str);

    enum ThreadPostType
    {
        THREAD_NONE = 0,
        THREAD_TOP = 1,
        THREAD_PARENT = 2,
        THREAD_ENTRY = 4, // Post that was selected to retrieve the thread
        THREAD_FIRST_DIRECT_CHILD = 8,
        THREAD_DIRECT_CHILD = 16,
        THREAD_CHILD = 32,
        THREAD_LEAF = 64
    };
    Q_ENUM(ThreadPostType);

    enum ReplyRestriction
    {
        REPLY_RESTRICTION_NONE = 0,
        REPLY_RESTRICTION_UNKNOWN = 1,
        REPLY_RESTRICTION_NOBODY = 2,
        REPLY_RESTRICTION_MENTIONED = 4,
        REPLY_RESTRICTION_FOLLOWING = 8,
        REPLY_RESTRICTION_LIST = 16,
        REPLY_RESTRICTION_FOLLOWER = 32
    };
    Q_ENUM(ReplyRestriction)

    enum StatusLevel
    {
        STATUS_LEVEL_INFO = 0,
        STATUS_LEVEL_ERROR
    };
    Q_ENUM(StatusLevel)

    enum NotificationReason
    {
        // Must match ATProto::AppBskyNotification::NotificationReason
        NOTIFICATION_REASON_LIKE = int(ATProto::AppBskyNotification::NotificationReason::LIKE),
        NOTIFICATION_REASON_REPOST = int(ATProto::AppBskyNotification::NotificationReason::REPOST),
        NOTIFICATION_REASON_FOLLOW = int(ATProto::AppBskyNotification::NotificationReason::FOLLOW),
        NOTIFICATION_REASON_MENTION = int(ATProto::AppBskyNotification::NotificationReason::MENTION),
        NOTIFICATION_REASON_REPLY = int(ATProto::AppBskyNotification::NotificationReason::REPLY),
        NOTIFICATION_REASON_QUOTE = int(ATProto::AppBskyNotification::NotificationReason::QUOTE),
        NOTIFICATION_REASON_STARTERPACK_JOINED = int(ATProto::AppBskyNotification::NotificationReason::STARTERPACK_JOINED),
        NOTIFICATION_REASON_VERIFIED = int(ATProto::AppBskyNotification::NotificationReason::VERIFIED),
        NOTIFICATION_REASON_UNVERIFIED = int(ATProto::AppBskyNotification::NotificationReason::UNVERIFIED),
        NOTIFICATION_REASON_LIKE_VIA_REPOST = int(ATProto::AppBskyNotification::NotificationReason::LIKE_VIA_REPOST),
        NOTIFICATION_REASON_REPOST_VIA_REPOST = int(ATProto::AppBskyNotification::NotificationReason::REPOST_VIA_REPOST),
        NOTIFICATION_REASON_SUBSCRIBED_POST = int(ATProto::AppBskyNotification::NotificationReason::SUBSCRIBED_POST),
        NOTIFICATION_REASON_UNKNOWN = int(ATProto::AppBskyNotification::NotificationReason::UNKNOWN),

        // Additional reasons
        NOTIFICATION_REASON_INVITE_CODE_USED,
        NOTIFICATION_REASON_DIRECT_MESSAGE,
        NOTIFICATION_REASON_DIRECT_MESSAGE_REACTION,
        NOTIFICATION_REASON_NEW_LABELS
    };
    Q_ENUM(NotificationReason)

    enum NotifcationChatIncludeType
    {
        NOTIFCATION_CHAT_INCLUDE_ALL = int(ATProto::AppBskyNotification::ChatPreference::IncludeType::ALL),
        NOTIFCATION_CHAT_INCLUDE_ACCEPTED = int(ATProto::AppBskyNotification::ChatPreference::IncludeType::ACCEPTED),
        NOTIFCATION_CHAT_INCLUDE_UNKNWON = int(ATProto::AppBskyNotification::ChatPreference::IncludeType::UNKNOWN)
    };
    Q_ENUM(NotifcationChatIncludeType)

    enum NotifcationFilterIncludeType
    {
        NOTIFICATION_FILTER_INCLUDE_ALL = int (ATProto::AppBskyNotification::FilterablePreference::IncludeType::ALL),
        NOTIFICATION_FILTER_INCLUDE_FOLLOWS = int (ATProto::AppBskyNotification::FilterablePreference::IncludeType::FOLLOWS),
        NOTIFICATION_FILTER_INCLUDE_UNKNOWN = int (ATProto::AppBskyNotification::FilterablePreference::IncludeType::UNKNOWN)
    };
    Q_ENUM(NotifcationFilterIncludeType)

    enum AllowActivitySubscriptionsType
    {
        ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS = int (ATProto::AppBskyActor::AllowSubscriptionsType::FOLLOWERS),
        ALLOW_ACTIVITY_SUBSCRIPTIONS_MUTUALS = int (ATProto::AppBskyActor::AllowSubscriptionsType::MUTUALS),
        ALLOW_ACTIVITY_SUBSCRIPTIONS_NONE = int (ATProto::AppBskyActor::AllowSubscriptionsType::NONE)
    };
    Q_ENUM(AllowActivitySubscriptionsType)

    enum AuthorListType
    {
        AUTHOR_LIST_FOLLOWS = 0,
        AUTHOR_LIST_FOLLOWERS,
        AUTHOR_LIST_KNOWN_FOLLOWERS,
        AUTHOR_LIST_BLOCKS,
        AUTHOR_LIST_MUTES,
        AUTHOR_LIST_LIKES,
        AUTHOR_LIST_REPOSTS,
        AUTHOR_LIST_SEARCH_RESULTS,
        AUTHOR_LIST_LIST_MEMBERS,
        AUTHOR_LIST_SUGGESTIONS,
        AUTHOR_LIST_LABELERS,
        AUTHOR_LIST_ACTIVITY_SUBSCRIPTIONS,
        AUTHOR_LIST_ACTIVE_FOLLOWS
    };
    Q_ENUM(AuthorListType)

    enum ContentVisibility
    {
        CONTENT_VISIBILITY_SHOW = 0,
        CONTENT_VISIBILITY_WARN_MEDIA = 1,
        CONTENT_VISIBILITY_WARN_POST = 2,
        CONTENT_VISIBILITY_HIDE_MEDIA = 3,
        CONTENT_VISIBILITY_HIDE_POST = 4,

        CONTENT_VISIBILITY_LAST = CONTENT_VISIBILITY_HIDE_POST
    };
    Q_ENUM(ContentVisibility)

    enum ContentPrefVisibility
    {
        CONTENT_PREF_VISIBILITY_SHOW = (int)ATProto::AppBskyActor::ContentLabelPref::Visibility::SHOW,
        CONTENT_PREF_VISIBILITY_WARN = (int)ATProto::AppBskyActor::ContentLabelPref::Visibility::WARN,
        CONTENT_PREF_VISIBILITY_HIDE = (int)ATProto::AppBskyActor::ContentLabelPref::Visibility::HIDE
    };
    Q_ENUM(ContentPrefVisibility)

    static ContentPrefVisibility toContentPrefVisibility(ContentVisibility);

    enum LabelTarget
    {
        LABEL_TARGET_CONTENT,
        LABEL_TARGET_MEDIA
    };
    Q_ENUM(LabelTarget)

    enum LabelSeverity
    {
        LABEL_SEVERITY_NONE,
        LABEL_SEVERITY_INFO,
        LABEL_SEVERITY_ALERT
    };
    Q_ENUM(LabelSeverity)

    enum ReportReasonType
    {
        REPORT_REASON_TYPE_NULL = -1,
        REPORT_REASON_TYPE_SPAM = (int)ATProto::ComATProtoModeration::ReasonType::SPAM,
        REPORT_REASON_TYPE_VIOLATION = (int)ATProto::ComATProtoModeration::ReasonType::VIOLATION,
        REPORT_REASON_TYPE_MISLEADING = (int)ATProto::ComATProtoModeration::ReasonType::MISLEADING,
        REPORT_REASON_TYPE_SEXUAL = (int)ATProto::ComATProtoModeration::ReasonType::SEXUAL,
        REPORT_REASON_TYPE_RUDE = (int)ATProto::ComATProtoModeration::ReasonType::RUDE,
        REPORT_REASON_TYPE_OTHER = (int)ATProto::ComATProtoModeration::ReasonType::OTHER,
        REPORT_REASON_TYPE_APPEAL = (int)ATProto::ComATProtoModeration::ReasonType::APPEAL
    };
    Q_ENUM(ReportReasonType)

    enum ReportTarget
    {
        REPORT_TARGET_POST,
        REPORT_TARGET_ACCOUNT,
        REPORT_TARGET_FEED,
        REPORT_TARGET_LIST,
        REPORT_TARGET_STARTERPACK,
        REPORT_TARGET_DIRECT_MESSAGE
    };
    Q_ENUM(ReportTarget)

    enum MutedPostReason
    {
        MUTED_POST_NONE = 0,
        MUTED_POST_AUTHOR,
        MUTED_POST_WORDS
    };
    Q_ENUM(MutedPostReason)

    enum ListPurpose
    {
        LIST_PURPOSE_MOD = (int)ATProto::AppBskyGraph::ListPurpose::MOD_LIST,
        LIST_PURPOSE_CURATE = (int)ATProto::AppBskyGraph::ListPurpose::CURATE_LIST,
        LIST_PURPOSE_REFERENCE = (int)ATProto::AppBskyGraph::ListPurpose::REFERENCE_LIST,
        LIST_PURPOSE_UNKNOWN = (int)ATProto::AppBskyGraph::ListPurpose::UNKNOWN
    };
    Q_ENUM(ListPurpose)

    enum ListType
    {
        LIST_TYPE_ALL,
        LIST_TYPE_BLOCKS,
        LIST_TYPE_MUTES,
        LIST_TYPE_SAVED
    };
    Q_ENUM(ListType)

    enum TripleBool
    {
        TRIPLE_BOOL_UNKNOWN,
        TRIPLE_BOOL_NO,
        TRIPLE_BOOL_YES
    };
    Q_ENUM(TripleBool)

    enum AuthorFeedFilter
    {
        AUTHOR_FEED_FILTER_NONE,
        AUTHOR_FEED_FILTER_POSTS,
        AUTHOR_FEED_FILTER_REPLIES,
        AUTHOR_FEED_FILTER_MEDIA,
        AUTHOR_FEED_FILTER_VIDEO
    };
    Q_ENUM(AuthorFeedFilter)

    enum PhotoType {
        PHOTO_TYPE_AVATAR,
        PHOTO_TYPE_BANNER
    };
    Q_ENUM(PhotoType)

    enum FontType
    {
        FONT_NORMAL,
        FONT_BOLD,
        FONT_ITALIC,
        FONT_STRIKETHROUGH,
        FONT_MONOSPACE,
        FONT_SMALL_CAPS,
        FONT_CURSIVE,
        FONT_FULLWIDTH,
        FONT_BUBBLE,
        FONT_SQUARE
    };
    Q_ENUM(FontType)

    enum AllowIncomingChat
    {
        ALLOW_INCOMING_CHAT_ALL = (int)ATProto::AppBskyActor::AllowIncomingType::ALL,
        ALLOW_INCOMING_CHAT_NONE = (int)ATProto::AppBskyActor::AllowIncomingType::NONE,
        ALLOW_INCOMING_CHAT_FOLLOWING = (int)ATProto::AppBskyActor::AllowIncomingType::FOLLOWING
    };
    Q_ENUM(AllowIncomingChat)

    enum ConvoStatus
    {
        CONVO_STATUS_REQUEST = (int)ATProto::ChatBskyConvo::ConvoStatus::REQUEST,
        CONVO_STATUS_ACCEPTED = (int)ATProto::ChatBskyConvo::ConvoStatus::ACCEPTED,
        CONVO_STATUS_UNKNOWN = (int)ATProto::ChatBskyConvo::ConvoStatus::UNKNOWN
    };
    Q_ENUM(ConvoStatus)

    enum ThreadStyle
    {
        THREAD_STYLE_BAR,
        THREAD_STYLE_LINE
    };
    Q_ENUM(ThreadStyle)

    enum Script
    {
        SCRIPT_LATIN = 0,
        SCRIPT_CHINESE = 1,
        SCRIPT_DEVANAGARI = 2,
        SCRIPT_JAPANESE = 3,
        SCRIPT_KOREAN = 4,

        SCRIPT_LAST = SCRIPT_KOREAN
    };
    Q_ENUM(Script)
    Q_INVOKABLE static QString scriptToString(Script script);

    enum LanguageSource
    {
        LANGUAGE_SOURCE_NONE,
        LANGUAGE_SOURCE_USER,
        LANGUAGE_SOURCE_AUTO
    };
    Q_ENUM(LanguageSource)


    enum FavoriteType
    {
        FAVORITE_FEED,
        FAVORITE_LIST,
        FAVORITE_SEARCH
    };
    Q_ENUM(FavoriteType)

    enum ActorTarget
    {
        ACTOR_TARGET_ALL = (int)ATProto::AppBskyActor::ActorTarget::ALL,
        ACTOR_TARGET_EXCLUDE_FOLLOWING = (int)ATProto::AppBskyActor::ActorTarget::EXCLUDE_FOLLOWING,
        ACTOR_TARGET_UNKNOWN = (int)ATProto::AppBskyActor::ActorTarget::UNKNOWN
    };
    Q_ENUM(ActorTarget)

    enum ContentMode
    {
        CONTENT_MODE_UNSPECIFIED = (int)ATProto::AppBskyFeed::ContentMode::UNSPECIFIED,
        CONTENT_MODE_VIDEO = (int)ATProto::AppBskyFeed::ContentMode::VIDEO,
        CONTENT_MODE_UNKNOWN = (int)ATProto::AppBskyFeed::ContentMode::UNKNOWN,

        // Not defined by ATProto
        CONTENT_MODE_MEDIA = 100,
        CONTENT_MODE_MEDIA_TILES = 101,
        CONTENT_MODE_VIDEO_TILES = 102,

        CONTENT_MODE_LAST = CONTENT_MODE_VIDEO_TILES
    };
    Q_ENUM(ContentMode)

    enum InsetsSide
    {
        INSETS_SIDE_TOP,
        INSETS_SIDE_BOTTOM,
        INSETS_SIDE_LEFT,
        INSETS_SIDE_RIGHT
    };
    Q_ENUM(InsetsSide)

    enum FeedType
    {
        FEED_GENERATOR,
        FEED_LIST,
        FEED_AUTHOR
    };
    Q_ENUM(FeedType)

    enum VideoQuality
    {
        VIDEO_QUALITY_HD,
        VIDEO_QUALITY_HD_WIFI,
        VIDEO_QUALITY_SD,

        VIDEO_QUALITY_LAST = VIDEO_QUALITY_SD
    };
    Q_ENUM(VideoQuality)

    enum FavoritesBarPosition
    {
        FAVORITES_BAR_POSITION_TOP,
        FAVORITES_BAR_POSITION_BOTTOM,
        FAVORITES_BAR_POSITION_NONE,

        FAVORITES_BAR_POSITION_LAST = FAVORITES_BAR_POSITION_NONE
    };
    Q_ENUM(FavoritesBarPosition)

    enum VerifiedStatus
    {
        VERIFIED_STATUS_VALID = (int)ATProto::AppBskyActor::VerifiedStatus::VALID,
        VERIFIED_STATUS_INVALID = (int)ATProto::AppBskyActor::VerifiedStatus::INVALID,
        VERIFIED_STATUS_NONE = (int)ATProto::AppBskyActor::VerifiedStatus::NONE,
        VERIFIED_STATUS_UNKNOWN = (int)ATProto::AppBskyActor::VerifiedStatus::UNKNOWN
    };
    Q_ENUM(VerifiedStatus)

    enum ActorStatus
    {
        ACTOR_STATUS_LIVE = (int)ATProto::AppBskyActor::ActorStatus::LIVE,
        ACTOR_STATUS_UNKNOWN = (int)ATProto::AppBskyActor::ActorStatus::UNKNOWN
    };
    Q_ENUM(ActorStatus)
};

}
