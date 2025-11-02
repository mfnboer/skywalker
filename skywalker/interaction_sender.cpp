// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "interaction_sender.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto SEND_INTERVAL = 31s;
static constexpr int MAX_INTERACTIONS = 150;

InteractionSender::InteractionSender(const QString& feedDid, ATProto::Client::SharedPtr& bsky, QObject* parent) :
    QObject(parent),
    mFeedDid(feedDid),
    mBsky(bsky)
{
    mSendTimer.setSingleShot(true);
    connect(&mSendTimer, &QTimer::timeout, this, [this]{ sendInteractions(); });
}

InteractionSender::~InteractionSender()
{
    qDebug() << "Delete interaction sender:" << mFeedDid;

    if (!mInteractions.empty())
        sendInteractions();
}

void InteractionSender::addInteraction(EventType event, const QString& postUri, const QString& feedContext)
{
    addInteraction(Interaction{event, postUri, feedContext});
}

void InteractionSender::removeInteraction(EventType event, const QString& postUri)
{
    removeInteraction(Interaction{event, postUri, ""});
}

void InteractionSender::addInteraction(const Interaction& interaction)
{
    if (mInteractions.size() >= MAX_INTERACTIONS)
    {
        qWarning() << "Maximum interactions:" << mInteractions.size() << "feedDid:" << mFeedDid;
        return;
    }

    if (mInteractions.empty())
        mSendTimer.start(SEND_INTERVAL);

    mInteractions.insert(interaction);
    qDebug() << "Added interaction:" << ATProto::AppBskyFeed::Interaction::eventTypeToString(interaction.mEvent) << interaction.mPostUri << "size:" << mInteractions.size() << "feedDid:" << mFeedDid;
}

void InteractionSender::removeInteraction(const Interaction& interaction)
{
    mInteractions.erase(interaction);
    qDebug() << "Removed interaction:" << ATProto::AppBskyFeed::Interaction::eventTypeToString(interaction.mEvent) << interaction.mPostUri << "size:" << mInteractions.size() << "feedDid:" << mFeedDid;

    if (mInteractions.empty())
        mSendTimer.stop();
}

void InteractionSender::sendInteractions()
{
    Q_ASSERT(mBsky);

    if (!mBsky)
        return;

    qDebug() << "Send interactions:" << mInteractions.size() << "feedDid:" << mFeedDid;

    if (mInteractions.empty())
        return;

    mBsky->sendInteractions(makeAtInteractionList(), mFeedDid,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            qDebug() << "Interactions sent:" << mInteractions.size() << "feedDid:" << mFeedDid;
            mInteractions.clear();
        },
        [this, presence=getPresence()](const QString& err, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to send interactions:" << err << " - " << msg << "feedDid:" << mFeedDid;
        });
}

ATProto::AppBskyFeed::Interaction::List InteractionSender::makeAtInteractionList() const
{
    ATProto::AppBskyFeed::Interaction::List atInteractionList;
    atInteractionList.reserve(mInteractions.size());

    for (const auto& interaction : mInteractions)
    {
        auto atInteraction = std::make_shared<ATProto::AppBskyFeed::Interaction>();
        atInteraction->mItem = interaction.mPostUri;
        atInteraction->mEvent = interaction.mEvent;
        atInteraction->mFeedContext = interaction.mFeedContext;
        qDebug() << "Interaction:" << atInteraction->toJson();
        atInteractionList.push_back(atInteraction);
    }

    return atInteractionList;
}

}
