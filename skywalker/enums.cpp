// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "enums.h"

namespace Skywalker {

QString QEnums::postTypeToString(PostType postType)
{
    static const std::unordered_map<PostType, QString> mapping = {
        { POST_STANDALONE, "standalone" },
        { POST_ROOT, "root" },
        { POST_REPLY, "reply" },
        { POST_LAST_REPLY, "lastReply" },
        { POST_THREAD, "thread" }
    };

    return ATProto::enumToString(postType, mapping, "unknown");
}

QEnums::PostType QEnums::stringToPostType(const QString& str)
{
    static const std::unordered_map<QString, PostType> mapping = {
        { "standalone", POST_STANDALONE },
        { "root", POST_ROOT },
        { "reply", POST_REPLY },
        { "lastReply", POST_LAST_REPLY },
        { "thread", POST_THREAD }
    };

    return ATProto::stringToEnum(str, mapping, POST_STANDALONE);
}

QString QEnums::foldedPostTypeToString(FoldedPostType foldedPostType)
{
    static const std::unordered_map<FoldedPostType, QString> mapping = {
        { FOLDED_POST_NONE, "none" },
        { FOLDED_POST_FIRST, "first" },
        { FOLDED_POST_SUBSEQUENT, "subsequent" }
    };

    return ATProto::enumToString(foldedPostType, mapping, "none");
}

QEnums::FoldedPostType QEnums::stringToFoldedPostType(const QString& str)
{
    static const std::unordered_map<QString, FoldedPostType> mapping = {
        { "none", FOLDED_POST_NONE },
        { "first", FOLDED_POST_FIRST },
        { "subsequent", FOLDED_POST_SUBSEQUENT }
    };

    return ATProto::stringToEnum(str, mapping, FOLDED_POST_NONE);
}

QEnums::ContentPrefVisibility QEnums::toContentPrefVisibility(ContentVisibility visibilty)
{
    switch (visibilty)
    {
    case CONTENT_VISIBILITY_SHOW:
        return CONTENT_PREF_VISIBILITY_SHOW;
    case CONTENT_VISIBILITY_WARN_MEDIA:
    case CONTENT_VISIBILITY_WARN_POST:
        return CONTENT_PREF_VISIBILITY_WARN;
    case CONTENT_VISIBILITY_HIDE_MEDIA:
    case CONTENT_VISIBILITY_HIDE_POST:
        return CONTENT_PREF_VISIBILITY_HIDE;
    }

    Q_ASSERT(false);
    return CONTENT_PREF_VISIBILITY_HIDE;
}

QString QEnums::scriptToString(Script script)
{
    static const std::unordered_map<Script, QString> mapping = {
        { SCRIPT_LATIN, tr("Latin") },
        { SCRIPT_CHINESE, tr("Chinese") },
        { SCRIPT_DEVANAGARI, tr("Devanagari") },
        { SCRIPT_JAPANESE, tr("Japanese") },
        { SCRIPT_KOREAN, tr("Korean") }
    };

    return ATProto::enumToString(script, mapping, "Latin");
}

QString QEnums::hideReasonToString(HideReasonType reason)
{
    static const std::unordered_map<HideReasonType, QString> mapping = {
        { HIDE_REASON_NONE, tr("None") },
        { HIDE_REASON_MUTED_AUTHOR, tr("Muted users") },
        { HIDE_REASON_REPOST_FROM_AUTHOR, tr("Muted reposts") },
        { HIDE_REASON_HIDE_FROM_FOLLOWING_FEED, tr("Hide from following feed") },
        { HIDE_REASON_LABEL, tr("Label") },
        { HIDE_REASON_MUTED_WORD, tr("Muted words") },
        { HIDE_REASON_HIDE_FOLLOWING_FROM_FEED, tr("Hide users you follow") },
        { HIDE_REASON_LANGUAGE, tr("Language") },
        { HIDE_REASON_QUOTE_BLOCKED_POST, tr("Quotes with blocked post") },
        { HIDE_REASON_REPLY_THREAD_UNFOLLOWED, tr("Replies in thread from not-followed users") },
        { HIDE_REASON_REPLY_TO_UNFOLLOWED, tr("Replies to not-followed users") },
        { HIDE_REASON_SELF_REPOST, tr("Self-reposts") },
        { HIDE_REASON_FOLLOWING_REPOST, tr("Reposted posts from followed users") },
        { HIDE_REASON_REPLY, tr("Replies") },
        { HIDE_REASON_REPOST, tr("Reposts") },
        { HIDE_REASON_QUOTE, tr("Quotes") },
        { HIDE_REASON_CONTENT_MODE, tr("Content imcompatible with feed") },
        { HIDE_REASON_ANY, tr("All filtered posts") }
    };

    return ATProto::enumToString(reason, mapping);
}

QString QEnums::replyOrderToString(ReplyOrder replyOrder)
{
    static const std::unordered_map<ReplyOrder, QString> mapping = {
        { REPLY_ORDER_SMART, tr("smart") },
        { REPLY_ORDER_OLDEST_FIRST, tr("oldest first") },
        { REPLY_ORDER_NEWEST_FIRST, tr("newest first") },
        { REPLY_ORDER_POPULARITY, tr("popularity") },
        { REPLY_ORDER_ENGAGEMENT, tr("engagement") }
    };

    return ATProto::enumToString(replyOrder, mapping);
}

QEnums::ContentMode QEnums::contentModeToFilterMode(ContentMode contentMode)
{
    switch (contentMode)
    {
    case CONTENT_MODE_MEDIA_TILES:
        return CONTENT_MODE_MEDIA;
    case CONTENT_MODE_VIDEO_TILES:
        return CONTENT_MODE_VIDEO;
    default:
        return contentMode;
    }
}

}
