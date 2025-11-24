// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class PostFeedContext
{
    Q_GADGET
    QML_VALUE_TYPE(postfeedcontext)

public:
    PostFeedContext() = default;
    PostFeedContext(const QString& replyFeedDid, const QString& replyFeedContext,
                    const QString& quoteFeedDid, const QString& quoteFeedContext);

    const QString& getReplyFeedDid() const { return mReplyFeedDid; }
    const QString& getReplyFeedContext() const { return mReplyFeedContext; }
    const QString& getQuoteFeedDid() const { return mQuoteFeedDid; }
    const QString& getQuoteFeedContext() const { return mQuoteFeedContext; }

private:
    QString mReplyFeedDid;
    QString mReplyFeedContext;
    QString mQuoteFeedDid;
    QString mQuoteFeedContext;
};

}
