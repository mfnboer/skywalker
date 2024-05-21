// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_view.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

MessageView::MessageView(const ATProto::ChatBskyConvo::MessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mText(msg.mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getLinkColor())),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::DeletedMessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mDeleted(true)
{
}

}
