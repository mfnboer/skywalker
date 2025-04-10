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
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getCurrentLinkColor())),
    mEmbed(msg.mEmbed),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt)
{
    initReactions(msg.mReactions);
}

MessageView::MessageView(const ATProto::ChatBskyConvo::DeletedMessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mDeleted(true)
{
}

MessageView::MessageView(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageType& msg)
{
    if (ATProto::isNullVariant(msg))
    {
        qWarning() << "Unknown message type";
        return;
    }

    const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::SharedPtr>(&msg);

    if (messageView)
    {
        init(*messageView);
        return;
    }

    const auto* deletedMessageView = std::get_if<ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr>(&msg);

    if (deletedMessageView)
    {
        init(*deletedMessageView);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

void MessageView::init(const ATProto::ChatBskyConvo::MessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mText = view->mText;
    mFormattedText = ATProto::RichTextMaster::getFormattedMessageText(*view, UserSettings::getCurrentLinkColor());
    mEmbed = view->mEmbed;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    initReactions(view->mReactions);
}

void MessageView::init(const ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    mDeleted = true;
}

void MessageView::initReactions(const ATProto::ChatBskyConvo::ReactionView::List& reactions)
{
    for (const auto& reaction : reactions)
    {
        const ReactionView reactionView(reaction);
        mReactions.push_back(reactionView);
    }

    // TODO TEST
    // for (int i = 0; i < 4; ++i)
    // {
    //     auto reaction = std::make_shared<ATProto::ChatBskyConvo::ReactionView>();
    //     reaction->mValue = "😀";
    //     reaction->mSender = std::make_shared<ATProto::ChatBskyConvo::ReactionViewSender>();
    //     reaction->mSender->mDid = mSenderDid;
    //     mReactions.push_back(ReactionView(reaction));
    // }

    // auto reaction = std::make_shared<ATProto::ChatBskyConvo::ReactionView>();
    // reaction->mValue = "❤️";
    // reaction->mSender = std::make_shared<ATProto::ChatBskyConvo::ReactionViewSender>();
    // reaction->mSender->mDid = mSenderDid;
    // mReactions.push_back(ReactionView(reaction));
}

ReactionView::List MessageView::getUniqueReactions(int maxReactions) const
{
    ReactionView::List result;
    std::unordered_set<QString> seenEmoji;

    for (const auto& reaction : mReactions)
    {
        if (seenEmoji.contains(reaction.getEmoji()))
            continue;

        result.push_back(reaction);

        if (result.size() >= maxReactions)
            break;

        seenEmoji.insert(reaction.getEmoji());
    }

    return result;
}

const RecordView MessageView::getEmbed() const
{
    if (!mEmbed)
        return {};

    RecordView recordView{*mEmbed};
    recordView.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
    recordView.setContentWarning("");
    return recordView;
}

}
