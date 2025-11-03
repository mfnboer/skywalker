// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "interaction_sender.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto SEND_INTERVAL = 31s;
static constexpr int MAX_INTERACTIONS = 150;
static constexpr int MAX_POSTS_ON_SCREEN = 200;
static constexpr auto MIN_POST_SEEN_DURATION = 800ms;

InteractionSender::InteractionSender(const QString& feedDid, ATProto::Client::SharedPtr bsky, QObject* parent) :
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

void InteractionSender::reportOnScreen(const QString& postUri)
{
    const auto now = std::chrono::high_resolution_clock::now();

    if (mPostUriOnScreen.contains(postUri))
    {
        const auto dt = now - mPostUriOnScreen[postUri];
        qDebug() << "Post already reported as on screen:" << postUri << "dt:" << dt / 1ms << "ms ago" << "feedDid:" << mFeedDid;
        return;
    }

    mPostUriOnScreen[postUri] = now;
    qDebug() << "Post on screen:" << postUri << "feedDid:" << mFeedDid;
    qDebug() << "On screen size:" << mPostUriOnScreen.size() << "feedDid:" << mFeedDid;

    // Protect against a future bug that could eat up all memory. There can only be
    // a handful of posts on screen. If this happens we clean the complete map.
    // No harm done, other than not reporting seen events. Most likely those
    // events are not valid anyway.
    if (mPostUriOnScreen.size() > MAX_POSTS_ON_SCREEN)
    {
        qWarning() << "Excessive number of posts on screen:" << mPostUriOnScreen.size() << "feedDid:" << mFeedDid;
        mPostUriOnScreen.clear();
    }
}

void InteractionSender::reportOffScreen(const QString& postUri, const QString& feedContext)
{
    auto it = mPostUriOnScreen.find(postUri);

    if (it == mPostUriOnScreen.end())
    {
        qDebug() << "Post not reported as on screen:" << postUri << "feedDid:" << mFeedDid;
        return;
    }

    const auto now = std::chrono::high_resolution_clock::now();
    const auto seenTimestamp = it->second;
    mPostUriOnScreen.erase(it);
    const auto dt = now - seenTimestamp;
    qDebug() << "Post:" << postUri << "off screen:" << dt / 1ms << "ms" << "feedDid:" << mFeedDid;
    qDebug() << "On screen size:" << mPostUriOnScreen.size() << "feedDid:" << mFeedDid;

    if (dt > MIN_POST_SEEN_DURATION)
        addInteraction(ATProto::AppBskyFeed::Interaction::EventType::InteractionSeen, postUri, feedContext);
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
