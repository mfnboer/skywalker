// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include <atproto/lib/client.h>
#include <atproto/lib/lexicon/app_bsky_feed.h>
#include <QHash>
#include <QObject>
#include <QTimer>

namespace Skywalker {

class InteractionSender : public QObject, public Presence
{
    Q_OBJECT

public:
    using Ptr = std::unique_ptr<InteractionSender>;
    using EventType = ATProto::AppBskyFeed::Interaction::EventType;

    InteractionSender(const QString& feedDid, ATProto::Client::SharedPtr& bsky, QObject* parent = nullptr);

    void addInteraction(EventType event, const QString& postUri, const QString& feedContext);
    void removeInteraction(EventType event, const QString& postUri);

private:
    struct Interaction
    {
        EventType mEvent;
        QString mPostUri;
        QString mFeedContext;

        bool operator==(const Interaction& other) const
        {
            return mEvent == other.mEvent && mPostUri == other.mPostUri;
        }

        struct Hash
        {
            size_t operator()(const Interaction& interaction) const
            {
                return qHashMulti(1325, (int)interaction.mEvent, interaction.mPostUri);
            }
        };
    };

    void addInteraction(const Interaction& interaction);
    void removeInteraction(const Interaction& interaction);
    void sendInteractions();
    ATProto::AppBskyFeed::Interaction::List makeAtInteractionList() const;

    QString mFeedDid;
    ATProto::Client::SharedPtr& mBsky;
    std::unordered_set<Interaction, Interaction::Hash> mInteractions;
    QTimer mSendTimer;
};


}
