// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

// enums that can be used in QML
class QEnums : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum PostType
    {
        POST_STANDALONE = 0,
        POST_ROOT,
        POST_REPLY,
        POST_LAST_REPLY,
        POST_THREAD
    };
    Q_ENUM(PostType)

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

    enum StatusLevel
    {
        STATUS_LEVEL_INFO = 0,
        STATUS_LEVEL_ERROR
    };
    Q_ENUM(StatusLevel)

    enum NotificationReason
    {
        NOTIFICATION_REASON_LIKE = 0,
        NOTIFICATION_REASON_REPOST,
        NOTIFICATION_REASON_FOLLOW,
        NOTIFICATION_REASON_MENTION,
        NOTIFICATION_REASON_REPLY,
        NOTIFICATION_REASON_QUOTE,
        NOTIFICATION_REASON_INVITE_CODE_USED,
        NOTIFICATION_REASON_UNKNOWN
    };
    Q_ENUM(NotificationReason)

    enum AuthorListType
    {
        AUTHOR_LIST_FOLLOWS = 0,
        AUTHOR_LIST_FOLLOWERS,
        AUTHOR_LIST_BLOCKS,
        AUTHOR_LIST_MUTES,
        AUTHOR_LIST_LIKES,
        AUTHOR_LIST_REPOSTS,
        AUTHOR_LIST_SEARCH_RESULTS
    };
    Q_ENUM(AuthorListType)

    enum ContentVisibility
    {
        CONTENT_VISIBILITY_SHOW = 0,
        CONTENT_VISIBILITY_WARN_MEDIA,
        CONTENT_VISIBILITY_WARN_POST,
        CONTENT_VISIBILITY_HIDE_MEDIA,
        CONTENT_VISIBILITY_HIDE_POST
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
};

}
