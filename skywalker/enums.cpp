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

    const auto it = mapping.find(postType);
    Q_ASSERT(it != mapping.end());

    if (it != mapping.end())
        return it->second;

    return "unknown";
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

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return POST_STANDALONE;
}

QString QEnums::foldedPostTypeToString(FoldedPostType foldedPostType)
{
    static const std::unordered_map<FoldedPostType, QString> mapping = {
        { FOLDED_POST_NONE, "none" },
        { FOLDED_POST_FIRST, "first" },
        { FOLDED_POST_SUBSEQUENT, "subsequent" }
    };

    const auto it = mapping.find(foldedPostType);
    Q_ASSERT(it != mapping.end());

    if (it != mapping.end())
        return it->second;

    return "none";
}

QEnums::FoldedPostType QEnums::stringToFoldedPostType(const QString& str)
{
    static const std::unordered_map<QString, FoldedPostType> mapping = {
        { "none", FOLDED_POST_NONE },
        { "first", FOLDED_POST_FIRST },
        { "subsequent", FOLDED_POST_SUBSEQUENT }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return FOLDED_POST_NONE;
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

}
