// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_view.h"
#include "unicode_fonts.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

MessageView::MessageView(const ATProto::ChatBskyConvo::MessageView& msg) :
    mId(msg.mId),
    mRev(msg.mRev),
    mText(msg.mText),
    mFormattedText(ATProto::RichTextMaster::getFormattedMessageText(msg, UserSettings::getCurrentLinkColor())),
    mTextMetaInfo(UnicodeFonts::getTextMetaInfo(mText, !msg.mFacets.empty())),
    mEmbed(msg.mEmbed),
    mSenderDid(msg.mSender->mDid),
    mSentAt(msg.mSentAt),
    mReplyTo(msg.mReplyTo)
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

    const auto* systemMessageView = std::get_if<ATProto::ChatBskyConvo::SystemMessageView::SharedPtr>(&msg);

    if (systemMessageView)
    {
        init(*systemMessageView);
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
    mTextMetaInfo = UnicodeFonts::getTextMetaInfo(mText, !view->mFacets.empty());
    mEmbed = view->mEmbed;
    mSenderDid = view->mSender->mDid;
    mSentAt = view->mSentAt;
    mEmbed = view->mEmbed;
    mReplyTo = view->mReplyTo;
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

void MessageView::init(const ATProto::ChatBskyConvo::SystemMessageView::SharedPtr& view)
{
    mId = view->mId;
    mRev = view->mRev;
    mSentAt = view->mSentAt;
    mFormattedText = "SYSTEM MESSAGE";
    // TODO
}

void MessageView::initReactions(const ATProto::ChatBskyConvo::ReactionView::List& reactions)
{
    for (const auto& reaction : reactions)
    {
        const ReactionView reactionView(reaction);
        mReactions.push_back(reactionView);
    }
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

    // TODO: JoinLinkView
    if (!ATProto::holdsNonNull<ATProto::AppBskyEmbed::RecordView::SharedPtr>(*mEmbed))
        return {};

    const auto view = std::get<ATProto::AppBskyEmbed::RecordView::SharedPtr>(*mEmbed);

    if (!view)
    {
        qWarning() << "No record view";
        return {};
    }

    RecordView recordView{*view};
    recordView.setContentVisibility(QEnums::CONTENT_VISIBILITY_SHOW);
    recordView.setContentWarning("");
    return recordView;
}

MessageView MessageView::getReplyTo() const
{
    if (!mReplyTo)
        return {};

    return std::visit([](auto&& view){ return MessageView(*view); }, *mReplyTo);
}

}
