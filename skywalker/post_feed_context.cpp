// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "post_feed_context.h"

namespace Skywalker {

PostFeedContext::PostFeedContext(const QString& replyFeedDid, const QString& replyFeedContext,
                                 const QString& quoteFeedDid, const QString& quoteFeedContext) :
    mReplyFeedDid(replyFeedDid),
    mReplyFeedContext(replyFeedContext),
    mQuoteFeedDid(quoteFeedDid),
    mQuoteFeedContext(quoteFeedContext)
{
}

}
