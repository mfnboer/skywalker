// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "chat_profile.h"
#include "utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto MESSAGES_UPDATE_INTERVAL = 9s;
static constexpr auto CONVOS_UPDATE_INTERVAL = 31s;
static constexpr char const* DM_ACCESS_ERROR = "Your APP password does not allow access to your direct messages. Create a new APP password that allows access.";

Chat::Chat(ATProto::Client::SharedPtr& bsky, const QString& userDid,
           const IProfileStore& mutedReposts, const IProfileStore& timelineHide,
           const ContentFilter& contentFilter,
           FollowsActivityStore& followsActivityStore, QObject* parent) :
    QObject(parent),
    mPresence(std::make_unique<Presence>()),
    mBsky(bsky),
    mUserDid(userDid),
    mMutedReposts(mutedReposts),
    mTimelineHide(timelineHide),
    mContentFilter(contentFilter),
    mFollowsActivityStore(followsActivityStore),
    mAcceptedConvoListModel(userDid, mFollowsActivityStore, this),
    mRequestConvoListModel(userDid, mFollowsActivityStore, this)
{
    connect(&mMessagesUpdateTimer, &QTimer::timeout, this, [this]{ updateMessages(); });
    connect(&mConvosUnreadUpdateTimer, &QTimer::timeout, this, [this]{ getConvosUnreadCounts(); });
    connect(&mAcceptedConvoListModel, &ConvoListModel::unreadCountChanged, this, [this]{ updateTotalUnreadCount(); });
    connect(&mRequestConvoListModel, &ConvoListModel::unreadCountChanged, this, [this]{ updateTotalUnreadCount(); });
}

void Chat::reset()
{
    qDebug() << "Reset chat";
    stopMessagesUpdateTimer();
    stopConvosUnreadUpdateTimer();

    mAcceptedConvoListModel.clear();
    mAcceptedConvoListModel.setGetConvosInProgress(false);
    mAcceptedConvoListModel.setLoaded(false);

    mRequestConvoListModel.clear();
    mRequestConvoListModel.setGetConvosInProgress(false);
    mRequestConvoListModel.setLoaded(false);

    mMessageListModels.clear();
    mConvoIdUpdatingMessages.clear();
    setUnreadCount(QEnums::CONVO_STATUS_ACCEPTED, 0);
    setUnreadCount(QEnums::CONVO_STATUS_REQUEST, 0);
    setStartConvoInProgress(false);
    setMessagesInProgress(false);
    mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
    mAllowGroupInvites = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
    mChatMaster = nullptr;
    mPostMaster = nullptr;
    mPresence = std::make_unique<Presence>();
}

void Chat::initSettings()
{
    qDebug() << "Init settings";
    Q_ASSERT(mBsky);

    if (!chatMaster())
        return;

    chatMaster()->getDeclaration(mUserDid,
        [this, presence=*mPresence](ATProto::ChatBskyActor::Declaration::SharedPtr declaration){
            if (!presence)
                return;

            mAllowIncomingChat = (QEnums::AllowIncomingChat)declaration->mAllowIncoming;
            mAllowGroupInvites = (QEnums::AllowIncomingChat)declaration->mAllowGroupInvites.value_or(declaration->mAllowIncoming);

            qDebug() << "Allow incoming chat:" << mAllowIncomingChat << "group:" << mAllowGroupInvites;
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to get chat settings:" << error << "-" << msg;
            mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
            mAllowGroupInvites = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
        });
}

void Chat::updateSettings(QEnums::AllowIncomingChat allowIncoming, QEnums::AllowIncomingChat allowGroupInvites)
{
    qDebug() << "Update settings";
    Q_ASSERT(mBsky);

    if (!chatMaster())
        return;

    ATProto::ChatBskyActor::Declaration declaration;
    declaration.mAllowIncoming = (ATProto::AppBskyActor::AllowIncomingType)allowIncoming;
    declaration.mAllowGroupInvites = (ATProto::AppBskyActor::AllowIncomingType)allowGroupInvites;

    chatMaster()->updateDeclaration(mUserDid, declaration,
        [this, presence=*mPresence, allowIncoming, allowGroupInvites]{
            if (!presence)
                return;

            mAllowIncomingChat = allowIncoming;
            mAllowGroupInvites = allowGroupInvites;
            qDebug() << "Allow incoming chat:" << mAllowIncomingChat << "group:" << mAllowGroupInvites;
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to update chat settings:" << error << "-" << msg;
            emit settingsFailed(msg);
        });
}

ATProto::ChatMaster* Chat::chatMaster()
{
    if (!mChatMaster)
    {
        if (mBsky)
            mChatMaster = std::make_unique<ATProto::ChatMaster>(*mBsky);
        else
            qWarning() << "Bsky client not yet created";
    }

    return mChatMaster.get();
}

ATProto::PostMaster* Chat::postMaster()
{
    if (!mPostMaster)
    {
        if (mBsky)
            mPostMaster = std::make_unique<ATProto::PostMaster>(*mBsky);
        else
            qWarning() << "Bsky client not yet created";
    }

    return mPostMaster.get();
}

QString Chat::getLastRev() const
{
    const auto lastRevAccepted = mAcceptedConvoListModel.getLastRevIncludingReactions();
    const auto lastRevRequested = mRequestConvoListModel.getLastRevIncludingReactions();

    return std::max(lastRevAccepted, lastRevRequested);
}

void Chat::start()
{
    startConvosUnreadUpdateTimer();
    getConvosUnreadCounts();
}

void Chat::getConvos(QEnums::ConvoStatus status, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get convos:" << status << "cursor:" << cursor;
    auto* model = getConvoListModel(status);

    if (!model)
    {
        qWarning() << "No model:" << status;
        return;
    }

    if (model->isGetConvosInProgress())
    {
        qDebug() << "Get convos still in progress:" << status;
        return;
    }

    model->setGetConvosInProgress(true);

    switch (status)
    {
    case QEnums::CONVO_STATUS_ACCEPTED:
        getAcceptedConvos(cursor);
        break;
    case QEnums::CONVO_STATUS_REQUEST:
        getRequestedConvos(cursor);
        break;
    case QEnums::CONVO_STATUS_UNKNOWN:
        qWarning() << "Unknown convo status";
        break;
    }
}

void Chat::getAcceptedConvos(const QString& cursor)
{
    mBsky->listConvos({}, false, ATProto::ChatBskyConvo::ConvoStatus::ACCEPTED, {}, {}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, cursor](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Get accepted convos ok";
            auto* model = getConvoListModel(QEnums::CONVO_STATUS_ACCEPTED);

            if (!model)
                return;

            if (cursor.isEmpty())
                model->clear();

            model->addConvos(output->mConvos, output->mCursor.value_or(""));
            model->setLoaded(true);
            model->setGetConvosInProgress(false);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getConvoListModel(QEnums::CONVO_STATUS_ACCEPTED);

            if (!model)
                return;

            qDebug() << "getConvos FAILED:" << error << " - " << msg;
            model->setGetConvosInProgress(false);

            if (error == ATProto::ATProtoErrorMsg::INVALID_TOKEN)
                emit failure(DM_ACCESS_ERROR);
            else
                emit failure(msg);
        }
    );
}

void Chat::getRequestedConvos(const QString& cursor)
{
    mBsky->listConvoRequests({}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, cursor](ATProto::ChatBskyConvo::ConvoRequestListOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Get accepted convos ok";
            auto* model = getConvoListModel(QEnums::CONVO_STATUS_REQUEST);

            if (!model)
                return;

            if (cursor.isEmpty())
                model->clear();

            model->addConvos(output->mRequests, output->mCursor.value_or(""));
            model->setLoaded(true);
            model->setGetConvosInProgress(false);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getConvoListModel(QEnums::CONVO_STATUS_REQUEST);

            if (!model)
                return;

            qDebug() << "getConvos FAILED:" << error << " - " << msg;
            model->setGetConvosInProgress(false);

            if (error == ATProto::ATProtoErrorMsg::INVALID_TOKEN)
                emit failure(DM_ACCESS_ERROR);
            else
                emit failure(msg);
        }
    );
}

void Chat::getConvosNextPage(QEnums::ConvoStatus status)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    const QString& cursor = model->getCursor();

    if(cursor.isEmpty())
    {
        qDebug() << "Last page reached:" << status;
        return;
    }

    getConvos(status, cursor);
}

QString Chat::getLastRevIncludingReactions(ConvoListModel* model, ATProto::ChatBskyConvo::ConvoView::List& convos)
{
    QString lastRev = "";

    for (auto& convo : convos)
    {
        if (convo->mRev > lastRev)
            lastRev = convo->mRev;

        if (convo->mLastReaction)
        {
            auto msgAndReaction = std::get<ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr>(*convo->mLastReaction);

            if (msgAndReaction->mMessageView->mRev > lastRev)
                lastRev = msgAndReaction->mMessageView->mRev;

            if (msgAndReaction->mMessageView->mRev > convo->mRev)
            {
                const ConvoView* existingConvo = model->getConvo(convo->mId);

                // HACK: Bluesky does not update the rev of the convo on a new reaction,
                // nor does it keep track of unseen reactions. By incrementing the unread
                // count here we can alert the user.
                if (existingConvo && msgAndReaction->mMessageView->mRev > existingConvo->getRevIncludingReactions())
                    convo->mUnreadCount++;
            }
        }
    }

    return lastRev;
}

// TODO: old way to get unread accounts. Expensive and does not work to get unread join requests for approval
void Chat::updateConvos(QEnums::ConvoStatus status)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update convos:" << status;

    mBsky->listConvos({}, false, ATProto::ChatBskyConvo::ConvoStatus(status), {}, {}, {},
        [this, presence=*mPresence, status](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Got convos";

            if (output->mConvos.empty())
            {
                qDebug() << "No convos:" << status;
                return;
            }

            auto* model = getConvoListModel(status);

            if (!model)
                return;

            // Before reactions we could simply compare the revisions of the first convo
            // in the list with the first convo in the model.
            // As Bluesky does not update the rev of the convo on new reactions, we have
            // to compare all convos...
            const QString rev = getLastRevIncludingReactions(model, output->mConvos);

            if (rev == model->getLastRevIncludingReactions())
            {
                qDebug() << "No updated convos, rev:" << rev << "status:" << status;
                return;
            }

            model->clear();
            setUnreadCount(status, 0);
            model->addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(status, *output);
            model->setLoaded(true);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateConvos FAILED:" << error << " - " << msg;
        }
        );
}

void Chat::getConvosUnreadCounts()
{
    Q_ASSERT(mBsky);
    qDebug() << "Get unread counts";

    mBsky->getConvoUnreadCounts(true,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoUnreadCountsOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Unread accecpted:" << output->mUnreadAcceptedConvos << "requests:" << output->mUnreadRequestConvos;
            mAcceptedConvoListModel.setUnreadCount(output->mUnreadAcceptedConvos);
            mRequestConvoListModel.setUnreadCount(output->mUnreadRequestConvos);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "getUnreadCounts FAILED:" << error << " - " << msg;
        });
}

void Chat::startConvoForMembers(const QStringList& dids, const QString& msg)
{
    Q_ASSERT(mBsky);
    qDebug() << "Start convo for members:" << dids;

    if (mStartConvoInProgress)
    {
        qDebug() << "Start convo still in progress";
        return;
    }

    const std::vector<QString> members(dids.begin(), dids.end());
    setStartConvoInProgress(true);

    mBsky->getConvoForMembers(members,
        [this, presence=*mPresence, msg](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setStartConvoInProgress(false);
            ConvoView convo(*output->mConvo, mUserDid);

            // A new convo with someone who does not follow you do not follow starts
            // in the REQUEST state. We set it to ACCEPTED as the first message typed will
            // accept the convo.
            // Theoretically a user may not type any message. In that case the convo will
            // move to the REQUEST inbox on the first update of the convo list models.
            convo.setStatus(QEnums::CONVO_STATUS_ACCEPTED);
            auto* model = getConvoListModel(convo.getStatus());

            if (model)
                model->insertConvo(convo);

            emit startConvoForMembersOk(convo, msg);
        },
        [this, presence=*mPresence](const QString& error, const QString& errorMsg){
            if (!presence)
                return;

            setStartConvoInProgress(false);
            qDebug() << "startConvoForMembers FAILED:" << error << " - " << errorMsg;

            if (error == ATProto::ATProtoErrorMsg::INVALID_TOKEN)
                emit startConvoForMembersFailed(DM_ACCESS_ERROR);
            else
                emit startConvoForMembersFailed(errorMsg);
        });
}

void Chat::startConvoForMember(const QString& did, const QString& msg)
{
    QStringList dids(did);
    startConvoForMembers(dids, msg);
}

void Chat::startConvoIfNotPresent(ConvoView convo)
{
    qDebug() << "Start convo if not present:" << convo.getId();
    const auto* existingConvo = getConvo(convo.getId());

    if (existingConvo && existingConvo->getStatus() == QEnums::CONVO_STATUS_ACCEPTED)
    {
        qDebug() << "Convo already exists:" << convo.getId();
        return;
    }

    qDebug() << "Insert convo as accepted:" << convo.getId();
    mRequestConvoListModel.deleteConvo(convo.getId());
    convo.setStatus(QEnums::CONVO_STATUS_ACCEPTED);
    mAcceptedConvoListModel.insertConvo(convo);
}

void Chat::acceptConvo(const ConvoView& convo)
{
    Q_ASSERT(mBsky);
    qDebug() << "Accept convo:" << convo.getId();

    if (mAcceptConvoInProgress)
    {
        qDebug() << "Accepting still in progress";
        return;
    }

    setAcceptConvoInProgress(true);

    mBsky->acceptConvo(convo.getId(),
        [this, presence=*mPresence, convo](ATProto::ChatBskyConvo::AcceptConvoOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Accepted convo:" << convo.getId();
            setAcceptConvoInProgress(false);

            if (output->mRev)
            {
                qDebug() << "Move convo to accepted:" << convo.getId() << "oldRev:" << convo.getRev() << "newRev:" << *output->mRev;
                ConvoView acceptedConvo(convo);
                acceptedConvo.setStatus(QEnums::CONVO_STATUS_ACCEPTED);
                acceptedConvo.setRev(*output->mRev);
                acceptedConvo.clearUnreadCount();
                mRequestConvoListModel.deleteConvo(convo.getId());
                mAcceptedConvoListModel.insertConvo(acceptedConvo);
                emit acceptConvoOk(acceptedConvo);
            }
            else
            {
                qDebug() << "Convo was already accepted";
                emit acceptConvoOk(convo);
            }
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "acceptConvo FAILED:" << error << " - " << msg;
            setAcceptConvoInProgress(false);
            emit failure(msg);
        });
}

void Chat::leaveConvo(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Leave convo:" << convoId;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Leave convo still in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->leaveConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::LeaveConvoOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Left convo:" << output->mConvoId;
            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.deleteConvo(output->mConvoId);
            mRequestConvoListModel.deleteConvo(output->mConvoId);
            emit leaveConvoOk();
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "leaveConvo FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::muteConvo(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Mute convo:" << convoId;

    mBsky->muteConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            updateConvoInModel(*output->mConvo);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "muteConvo FAILED:" << error << " - " << msg;
            emit failure(msg);
        });
}

void Chat::unmuteConvo(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Unmute convo:" << convoId;

    mBsky->unmuteConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            updateConvoInModel(*output->mConvo);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "unmuteConvo FAILED:" << error << " - " << msg;
            emit failure(msg);
        });
}

bool Chat::convosLoaded(QEnums::ConvoStatus status) const
{
    const auto* model = getConvoListModel(status);
    return model ? model->isLoaded() : false;
}

bool Chat::convosLoaded() const
{
    return convosLoaded(QEnums::CONVO_STATUS_ACCEPTED) || convosLoaded(QEnums::CONVO_STATUS_REQUEST);
}

const ConvoView* Chat::getConvo(const QString& convoId) const
{
    const ConvoView* convo = mAcceptedConvoListModel.getConvo(convoId);

    if (!convo)
        convo = mRequestConvoListModel.getConvo(convoId);

    return convo;
}

ConvoListModel* Chat::getConvoListModel(QEnums::ConvoStatus status) const
{
    switch (status)
    {
    case QEnums::CONVO_STATUS_REQUEST:
        return const_cast<ConvoListModel*>(&mRequestConvoListModel);
    case QEnums::CONVO_STATUS_ACCEPTED:
        return const_cast<ConvoListModel*>(&mAcceptedConvoListModel);
    case QEnums::CONVO_STATUS_UNKNOWN:
        break;
    }

    Q_ASSERT(false);
    qWarning() << "Invalid status:" << (int)status;
    return nullptr;
}

void Chat::updateConvoInModel(const ATProto::ChatBskyConvo::ConvoView& convo)
{
    mAcceptedConvoListModel.updateConvo(convo);
    mRequestConvoListModel.updateConvo(convo);
}

void Chat::updateTotalUnreadCount()
{
    const int totalUnread = mAcceptedConvoListModel.getUnreadCount() + mRequestConvoListModel.getUnreadCount();

    if (mUnreadCount != totalUnread)
    {
        mUnreadCount = totalUnread;
        emit unreadCountChanged();
    }
}

void Chat::setUnreadCount(QEnums::ConvoStatus status, int unread)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    model->setUnreadCount(unread);
}

// TODO: model unread count is now the number of unread convos set by getConvosUnreadCounts
void Chat::updateUnreadCount(QEnums::ConvoStatus status, const ATProto::ChatBskyConvo::ConvoListOutput& output)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    model->updateUnreadCount(output);
}

// TODO: model unread count is now the number of unread convos set by getConvosUnreadCounts
void Chat::updateUnreadCount(QEnums::ConvoStatus status, const ATProto::ChatBskyConvo::ConvoRequestListOutput& output)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    model->updateUnreadCount(output);
}

void Chat::setStartConvoInProgress(bool inProgress)
{
    if (inProgress != mStartConvoInProgress)
    {
        mStartConvoInProgress = inProgress;
        emit startConvoInProgressChanged();
    }
}

void Chat::setAcceptConvoInProgress(bool inProgress)
{
    if (inProgress != mAcceptConvoInProgress)
    {
        mAcceptConvoInProgress = inProgress;
        emit acceptConvoInProgressChanged();
    }
}

void Chat::setConvoUpdateInProgress(bool inProgress)
{
    if (inProgress != mConvoUpdateInProgress)
    {
        mConvoUpdateInProgress = inProgress;
        emit convoUpdateInProgressChanged();
    }
}

void Chat::setMessagesInProgress(bool inProgress)
{
    if (inProgress != mGetMessagesInProgress)
    {
        mGetMessagesInProgress = inProgress;
        emit getMessagesInProgressChanged();
    }
}

ChatAuthorListModel* Chat::getConvoAuthorListModel(QEnums::ChatAuthorListType type, const QString& convoId)
{
    const auto* convo = getConvo(convoId);

    if (!convo)
    {
        qWarning() << "Cannot find convo:" << convoId;
        return nullptr;
    }

    auto& model = mConvoAuthorListModels[{type, convoId}];

    if (!model)
    {
        qDebug() << "Create convo author list model for convo:" << convoId << "type:" << type;
        model = std::make_unique<ChatAuthorListModel>(
            type, mMutedReposts, mTimelineHide, mContentFilter, this);
    }

    return model.get();
}

void Chat::removeConvoAuthorListModel(QEnums::ChatAuthorListType type, const QString& convoId)
{
    qDebug() << "Delete convo author list model for convo:" << convoId << "type:" << type;
    mConvoAuthorListModels.erase({type, convoId});
}

void Chat::getConvoAuthors(QEnums::ChatAuthorListType type, const QString& convoId, const QString& cursor)
{
    switch (type)
    {
    case QEnums::CHAT_AUTHOR_LIST_MEMBERS:
        getConvoMembers(convoId, cursor);
        return;
    case QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS:
        getJoinRequests(convoId, cursor);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Unexpected type:" << type;
}

void Chat::getConvoAuthorsNextPage(QEnums::ChatAuthorListType type, const QString& convoId)
{
    auto* model = getConvoAuthorListModel(type, convoId);

    if (!model)
    {
        qDebug() << "Model already closed for convo:" << convoId;
        return;
    }

    const QString& cursor = model->getCursor();

    if(cursor.isEmpty())
    {
        qDebug() << "Last page reached:" << type;
        return;
    }

    getConvoAuthors(type, convoId, cursor);
}

void Chat::getConvoMembers(const QString& convoId, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get convo members, convoId:" << convoId << "cursor:" << cursor;

    auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId);

    if (!model)
    {
        qWarning() << "No model";
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get convo members still in progress";
        return;
    }

    model->setGetFeedInProgress(true);

    mBsky->getConvoMembers(convoId, {}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, convoId, cursor](ATProto::ChatBskyConvo::GetConvoMembersOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId);

            if (model)
            {
                if (cursor.isEmpty())
                    model->clear();

                model->addAuthors(output->mMembers, output->mCursor.value_or(""));
                model->setGetFeedInProgress(false);
            }
            else
            {
                qDebug() << "Model already closed for convo:" << convoId;
            }
        },
        [this, presence=*mPresence, convoId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getConvoMembers FAILED:" << error << " - " << msg;

            auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
        }
    );
}

void Chat::getJoinRequests(const QString& convoId, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get join requests, convoId:" << convoId << "cursor:" << cursor;

    auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS, convoId);

    if (!model)
    {
        qWarning() << "No model";
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get join requests still in progress";
        return;
    }

    model->setGetFeedInProgress(true);

    mBsky->listJoinRequests(convoId, {}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, convoId, cursor](ATProto::ChatBskyGroup::JoinRequestsOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS, convoId);

            if (model)
            {
                if (cursor.isEmpty())
                    model->clear();

                model->addJoinRequests(output->mRequests, output->mCursor.value_or(""));
                model->setGetFeedInProgress(false);
            }
            else
            {
                qDebug() << "Model already closed for convo:" << convoId;
            }
        },
        [this, presence=*mPresence, convoId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getJoinRequests FAILED:" << error << " - " << msg;

            auto* model = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS, convoId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
        }
    );
}

void Chat::removeMember(const QString& convoId, const QString& did)
{
    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->removeMembers(convoId, {did},
        [this, presence=*mPresence, convoId, did](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            if (mConvoAuthorListModels.contains({ QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId }))
            {
                auto& model = mConvoAuthorListModels[{ QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId }];
                model->deleteAuthor(did);
            }

            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "removeMember FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit removeMemberFailed(msg);
        });
}

void Chat::addMember(const QString& convoId, const QString& did)
{
    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->addMembers(convoId, {did},
        [this, presence=*mPresence, convoId, did](ATProto::ChatBskyGroup::AddMembersOutput::SharedPtr output){
            if (!presence)
                return;

            if (mConvoAuthorListModels.contains({ QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId }))
            {
                auto& model = mConvoAuthorListModels[{ QEnums::CHAT_AUTHOR_LIST_MEMBERS, convoId }];

                for (const auto& member : output->mAddedMebers)
                    model->prependAuthor(*member);
            }

            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addMember FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit removeMemberFailed(msg);
        });
}

void Chat::createGroupConvo(const QString& name)
{
    if (mStartConvoInProgress)
    {
        qDebug() << "Starting convo in progress";
        return;
    }

    setStartConvoInProgress(true);

    mBsky->createGroup({}, name,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setStartConvoInProgress(false);
            ConvoView convo(*output->mConvo, mUserDid);
            convo.setStatus(QEnums::CONVO_STATUS_ACCEPTED);
            auto* model = getConvoListModel(convo.getStatus());

            if (model)
                model->insertConvo(convo);

            auto* membersModel = getConvoAuthorListModel(QEnums::CHAT_AUTHOR_LIST_MEMBERS, convo.getId());

            if (membersModel && !output->mConvo->mMembers.empty())
                membersModel->prependAuthor(*output->mConvo->mMembers.front());

            emit createGroupConvoOk(convo);
        },
        [this, presence=*mPresence, name](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "create group convo FAILED:" << error << " - " << msg;
            setStartConvoInProgress(false);
            emit failure(tr("Could not create group %1: %2").arg(name, msg));
        });
}

void Chat::editGroupConvo(const QString& convoId, const QString& name)
{
    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->editGroup(convoId, name,
        [this, presence=*mPresence, convoId](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "edit group convo FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::lockGroupConvo(const QString& convoId)
{
    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->lockConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "lockConvo FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::unlockGroupConvo(const QString& convoId)
{
    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->unlockConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "unlockConvo FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::createJoinLink(const QString& convoId, QEnums::JoinRule joinRule, bool requireApproval)
{
    qDebug() << "Create join link:" << convoId << "rule:" << joinRule << "approval:" << requireApproval;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->createJoinLink(convoId, requireApproval, ATProto::ChatBskyGroup::JoinRule(joinRule),
        [this, presence=*mPresence, convoId](ATProto::ChatBskyGroup::JoinLinkOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            joinLinkUpdatedOk(convoId, output);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "createJoinLink FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::editJoinLink(const QString& convoId, QEnums::JoinRule joinRule, bool requireApproval)
{
    qDebug() << "Edit join link:" << convoId << "rule:" << joinRule << "approval:" << requireApproval;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->editJoinLink(convoId, requireApproval, ATProto::ChatBskyGroup::JoinRule(joinRule),
        [this, presence=*mPresence, convoId](ATProto::ChatBskyGroup::JoinLinkOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            joinLinkUpdatedOk(convoId, output);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "editJoinLink FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::disableJoinLink(const QString& convoId)
{
    qDebug() << "Disable join link:" << convoId;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->disableJoinLink(convoId,
        [this, presence=*mPresence, convoId](ATProto::ChatBskyGroup::JoinLinkOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            joinLinkUpdatedOk(convoId, output);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "disableJoinLink FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::enableJoinLink(const QString& convoId)
{
    qDebug() << "Enable join link:" << convoId;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->enableJoinLink(convoId,
        [this, presence=*mPresence, convoId](ATProto::ChatBskyGroup::JoinLinkOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            joinLinkUpdatedOk(convoId, output);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "disableJoinLink FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::joinLinkUpdatedOk(const QString& convoId, ATProto::ChatBskyGroup::JoinLinkOutput::SharedPtr output)
{
    qDebug() << "Join link updated:" << (QEnums::JoinRule)output->mJoinLink->mJoinRule << "approval:" << output->mJoinLink->mRequireApproval;
    auto* convo = mAcceptedConvoListModel.getConvo(convoId);

    if (convo)
    {
        auto atprotoGroupConvo = convo->getGroupConvo().getATProtoGroupConvo();

        if (atprotoGroupConvo)
        {
            atprotoGroupConvo->mJoinLink = output->mJoinLink;
            mAcceptedConvoListModel.updateConvo(*convo);
            emit convoUpdated(*convo);
        }
        else
        {
            qWarning() << "Missing group convo:" << convo->getId();
        }
    }
    else
    {
        qWarning() << "Convo not found:" << convoId;
    }
}

// TODO: cache
void Chat::getJoinLinkPreview(const QString& code)
{
    qDebug() << "Get join link preview:" << code;

    mBsky->getJoinLinkPreviews({code},
        [this, presence=*mPresence](ATProto::ChatBskyGroup::JoinLinkPreviewsOutput::SharedPtr output){
            if (!presence)
                return;

            if (output->mJoinLinkPreviews.size() != 1)
            {
                qWarning() << "Invalid join link previews:" << output->mJoinLinkPreviews.size();
                return;
            }

            const JoinLinkPreview preview =
                std::visit([](auto&& x){ return JoinLinkPreview(*x); }, output->mJoinLinkPreviews.front());

            emit joinLinkPreviewOk(preview);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getJoinLinkPreview FAILED:" << error << " - " << msg;
            emit failure(msg);
        });
}

void Chat::updateJoinRequestsRead(const QString& convoId)
{
    qDebug() << "Update join requests read convo:" << convoId;
    auto* convo = mAcceptedConvoListModel.getConvo(convoId);

    if (!convo)
    {
        qWarning() << "No convo:" << convoId;
        return;
    }

    if (convo->getGroupConvo().getUnreadJoinRequestCount() == 0)
    {
        qDebug() << "No unread join requests:" << convoId;
        return;
    }

    mBsky->updateJoinRequestsRead(convoId,
        [this, presence=*mPresence, convoId]{
            if (!presence)
                return;

            auto* convo = mAcceptedConvoListModel.getConvo(convoId);

            if (!convo)
                return;

            const bool hasUnreadMessages = convo->getUnreadCount() > 0;

            ConvoView updatedConvo(*convo);
            updatedConvo.getGroupConvo().clearUnreadJoinRequestCount();
            mAcceptedConvoListModel.updateConvo(updatedConvo);

            if (!updatedConvo.isMuted() && !hasUnreadMessages)
            {
                const int newUnreadCount = mAcceptedConvoListModel.getUnreadCount() - 1;
                setUnreadCount(QEnums::CONVO_STATUS_ACCEPTED, newUnreadCount);
            }

            emit convoUpdated(updatedConvo);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateJoinRequestsRead FAILED:" << error << " - " << msg;
        }
    );
}

void Chat::approveJoinRequest(const QString& convoId, const ChatBasicProfile& member)
{
    const QString& did = member.getBasicProfile().getDid();
    qDebug() << "Approve join request:" << convoId << "did:" << did;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->approveJoinRequest(convoId, did,
        [this, presence=*mPresence, member](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            const ConvoAuthorListKey key{QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS, output->mConvo->mId};

            if (mConvoAuthorListModels.contains(key))
                mConvoAuthorListModels[key]->deleteAuthor(member.getBasicProfile().getDid());

            mAcceptedConvoListModel.updateConvo(*output->mConvo);
            emit convoUpdated(ConvoView(*output->mConvo, mUserDid));

            const QString msg = tr("Added: %1").arg(member.getBasicProfile().getName());
            emit statusMessage(msg);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "approveJoinRequest FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::rejectJoinRequest(const QString& convoId, const ChatBasicProfile& member)
{
    const QString& did = member.getBasicProfile().getDid();
    qDebug() << "Reject join request:" << convoId << "did:" << did;

    if (mConvoUpdateInProgress)
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->rejectJoinRequest(convoId, did,
        [this, presence=*mPresence, convoId, member]{
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            const ConvoAuthorListKey key{QEnums::CHAT_AUTHOR_LIST_JOIN_REQUESTS, convoId};

            if (mConvoAuthorListModels.contains(key))
                mConvoAuthorListModels[key]->deleteAuthor(member.getBasicProfile().getDid());

            auto* convo = mAcceptedConvoListModel.getConvo(convoId);

            if (convo)
            {
                ConvoView updatedConvo(*convo);
                updatedConvo.getGroupConvo().decrementJoinRequestCount();
                mAcceptedConvoListModel.updateConvo(updatedConvo);
                emit convoUpdated(updatedConvo);
            }

            const QString msg = tr("Rejected: %1").arg(member.getBasicProfile().getName());
            emit statusMessage(msg);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "rejectJoinRequest FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

bool Chat::isRequestJoinInProgress(const QString& code) const
{
    return mRequestJoinInProgess.contains(code);
}

void Chat::setRequestJoinInProgress(const QString& code, bool inProgress)
{
    if (inProgress)
        mRequestJoinInProgess.insert(code);
    else
        mRequestJoinInProgess.erase(code);
}

void Chat::requestJoin(const JoinLinkPreview& joinLink)
{
    const QString& code = joinLink.getCode();
    qDebug() << "Request join:" << code << "name:" << joinLink.getName();

    if (isRequestJoinInProgress(code))
    {
        qDebug() << "Request join in progress:" << code;
        return;
    }

    setRequestJoinInProgress(code, true);

    mBsky->requestJoin(code,
        [this, presence=*mPresence, joinLink](ATProto::ChatBskyGroup::RequestJoinOutput::SharedPtr output){
            if (!presence)
                return;

            const QString& code = joinLink.getCode();
            setRequestJoinInProgress(code, false);

            switch (output->mStatus)
            {
            case ATProto::ChatBskyGroup::RequestJoinStatus::PENDING:
            {
                JoinLinkPreview pendingJoinLink(joinLink);
                pendingJoinLink.setRequestedAtToNow();
                emit requestJoinPending(pendingJoinLink);
                break;
            }
            case ATProto::ChatBskyGroup::RequestJoinStatus::JOINED:
            {
                if (!output->mConvo)
                {
                    qWarning() << "Convo missing:" << code;
                    emit requestJoinFailed(code, tr("You have joined, but the convo cannot be opened"));
                    return;
                }

                JoinLinkPreview joinedJoinLink(joinLink);
                joinedJoinLink.setConvoView(output->mConvo);
                emit requestJoinJoined(joinedJoinLink);
                return;
            }
            case ATProto::ChatBskyGroup::RequestJoinStatus::UNKNOWN:
                qWarning() << "Unexpected join request status:" << output->mRawStatus;
                emit requestJoinFailed(code, tr("Unexpected join request status: %1").arg(output->mRawStatus));
                return;
            }
        },
        [this, presence=*mPresence, code](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "requestJoin FAILED:" << error << " - " << msg;
            setRequestJoinInProgress(code, false);
            emit requestJoinFailed(code, msg);
        });
}

void Chat::withdrawJoinRequest(const QString convoId)
{
    qDebug() << "Withdraw join request:" << convoId;

    if (isConvoUpdateInProgress())
    {
        qDebug() << "Convo update in progress";
        return;
    }

    setConvoUpdateInProgress(true);

    mBsky->withdrawJoinRequest(convoId,
        [this, presence=*mPresence, convoId]{
            if (!presence)
                return;

            setConvoUpdateInProgress(false);
            mRequestConvoListModel.deleteConvo(convoId);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "withdrawJoinRequest FAILED:" << error << " - " << msg;
            setConvoUpdateInProgress(false);
            emit failure(msg);
        });
}

void Chat::withdrawJoinRequest(const JoinLinkPreview& joinLink)
{
    qDebug() << "Withdraw join request:" << joinLink.getConvoId() << "name:" << joinLink.getName();

    if (isRequestJoinInProgress(joinLink.getCode()))
    {
        qDebug() << "Request join in progress:" << joinLink.getCode();
        return;
    }

    setRequestJoinInProgress(joinLink.getCode(), true);

    mBsky->withdrawJoinRequest(joinLink.getConvoId(),
        [this, presence=*mPresence, joinLink]{
            if (!presence)
                return;

            setRequestJoinInProgress(joinLink.getCode(), false);
            mRequestConvoListModel.deleteConvo(joinLink.getConvoId());

            JoinLinkPreview withdrawnJoinLink(joinLink);
            withdrawnJoinLink.clearRequestedAt();
            emit withdrawJoinRequestOk(withdrawnJoinLink);
        },
        [this, presence=*mPresence, joinLink](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "withdrawJoinRequest FAILED:" << error << " - " << msg;
            setRequestJoinInProgress(joinLink.getCode(), false);
            emit withdrawJoinRequestFailed(joinLink.getCode(), msg);
        });
}

QString Chat::getJoinLinkCodeFromUri(const QString& uri)
{
    const QUrl url(uri);

    if (!url.isValid())
    {
        qWarning() << "Invalid url:" << uri;
        return {};
    }

    if (url.host() != "bsky.app")
    {
        qDebug() << "Not a join link:" << uri;
        return {};
    }

    const QString path = url.path();

    if (!path.startsWith("/chat/"))
    {
        qDebug() << "Not a join link:" << uri;
        return {};
    }

    const auto& pathSegments = path.split("/", Qt::SkipEmptyParts);

    if (pathSegments.size() != 2)
    {
        qDebug() << "Not a join link:" << uri;
        return {};
    }

    return pathSegments.back();
}

bool Chat::isJoinLinkUri(const QString& uri)
{
    const QString code = getJoinLinkCodeFromUri(uri);
    return (!code.isEmpty());
}

void Chat::updateBlockingUri(const QString& did, const QString& blockingUri)
{
    mAcceptedConvoListModel.updateBlockingUri(did, blockingUri);
    mRequestConvoListModel.updateBlockingUri(did, blockingUri);
}

void Chat::makeLocalModelChange(const std::function<void(LocalAuthorModelChanges*)>& update)
{
    for (auto& [_, model] : mConvoAuthorListModels)
        update(model.get());
}

MessageListModel* Chat::getMessageListModel(const QString& convoId)
{
    const auto* convo = getConvo(convoId);

    if (!convo)
    {
        qWarning() << "Cannot find convo:" << convoId;
        return nullptr;
    }

    auto& model = mMessageListModels[convoId];

    if (!model)
    {
        qDebug() << "Create message list model for convo:" << convoId;
        ChatBasicProfileList profiles;
        profiles = convo->getMembers();
        model = std::make_unique<MessageListModel>(mUserDid, profiles, mFollowsActivityStore, this);
        startMessagesUpdateTimer();
    }

    return model.get();
}

void Chat::removeMessageListModel(const QString& convoId)
{
    qDebug() << "Delete message list model for convo:" << convoId;
    mMessageListModels.erase(convoId);
    setMessagesUpdating(convoId, false);

    if (mMessageListModels.empty())
        stopMessagesUpdateTimer();
}

void Chat::getMessages(const QString& convoId, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get messages, convoId:" << convoId << "cursor:" << cursor;

    if (mGetMessagesInProgress)
    {
        qDebug() << "Get messages still in progress";
        return;
    }

    setMessagesInProgress(true);
    mBsky->getMessages(convoId, {}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, convoId, cursor](ATProto::ChatBskyConvo::GetMessagesOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = getMessageListModel(convoId);

            if (model)
            {
                if (cursor.isEmpty())
                    model->clear();

                model->addMessages(output->mMessages, output->mCursor.value_or(""));
            }
            else
            {
                qDebug() << "Model already closed for convo:" << convoId;
            }

            setMessagesInProgress(false);
            emit getMessagesOk(cursor);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getMessages FAILED:" << error << " - " << msg;
            setMessagesInProgress(false);
            emit getMessagesFailed(msg);
        }
    );
}

void Chat::getMessagesNextPage(const QString& convoId)
{
    auto* model = getMessageListModel(convoId);

    if (!model)
    {
        qDebug() << "Model already closed for convo:" << convoId;
        return;
    }

    const QString& cursor = model->getCursor();

    if(cursor.isEmpty())
    {
        qDebug() << "Last page reached";
        return;
    }

    getMessages(convoId, cursor);
}

void Chat::updateMessages(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update messages, convoId:" << convoId;

    if (isMessagesUpdating(convoId))
    {
        qDebug() << "Still updating messages:" << convoId;
        return;
    }

    setMessagesUpdating(convoId, true);

    mBsky->getMessages(convoId, {}, {},
        [this, presence=*mPresence, convoId](ATProto::ChatBskyConvo::GetMessagesOutput::SharedPtr output){
            if (!presence)
                return;

            setMessagesUpdating(convoId, false);
            auto* model = getMessageListModel(convoId);

            if (!model)
            {
                qDebug() << "Model already closed for convo:" << convoId;
                return;
            }

            model->updateMessages(output->mMessages, output->mCursor.value_or(""));
        },
        [this, presence=*mPresence, convoId](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "updateMessages FAILED:" << error << " - " << msg;
            setMessagesUpdating(convoId, false);
        });
}

void Chat::updateMessages()
{
    qDebug() << "Update messages";

    for (const auto& [convoId, _] : mMessageListModels)
        updateMessages(convoId);
}

void Chat::updateRead(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update read convo:" << convoId;

    auto status = QEnums::ConvoStatus::CONVO_STATUS_ACCEPTED;
    auto* convo = mAcceptedConvoListModel.getConvo(convoId);

    if (!convo)
    {
        status = QEnums::ConvoStatus::CONVO_STATUS_REQUEST;
        convo = mRequestConvoListModel.getConvo(convoId);
    }

    if (!convo)
    {
        qDebug() << "No convo";
        return;
    }

    const QString& lastReadMessageId = getLastReadMessageId(*convo);

    if (lastReadMessageId.isEmpty())
        return;

    mBsky->updateRead(convoId, lastReadMessageId,
        [this, presence=*mPresence, status](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = getConvoListModel(status);

            if (!model)
                return;

            model->updateConvo(*output->mConvo);

            int unreadJoinRequestCount = 0;

            if (output->mConvo->mKind && ATProto::holdsNonNull<ATProto::ChatBskyConvo::GroupConvo::SharedPtr>(*output->mConvo->mKind))
            {
                const auto& group = std::get<ATProto::ChatBskyConvo::GroupConvo::SharedPtr>(*output->mConvo->mKind);
                unreadJoinRequestCount = group->mUnreadJoinRequestCount.value_or(0);
            }

            if (!output->mConvo->mMuted && output->mConvo->mUnreadCount == 0 && unreadJoinRequestCount == 0)
            {
                const int newUnreadCount = model->getUnreadCount() - 1;
                setUnreadCount(status, newUnreadCount);
            }
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateRead FAILED:" << error << " - " << msg;
        }
    );
}

QString Chat::getLastReadMessageId(const ConvoView& convo) const
{
    auto it = mMessageListModels.find(convo.getId());

    if (it == mMessageListModels.end())
    {
        qDebug() << "No read messages";
        return {};
    }

    const auto& messageListModel = it->second;
    const MessageView* lastReadMessage = messageListModel->getLastMessage();
    const MessageView& lastConvoMessage = convo.getLastMessage();

    if (!lastReadMessage )
    {
        if (lastConvoMessage.isDeleted())
        {
            qDebug() << "All messages deleted, convo has a deleted last message";
            return lastConvoMessage.getId();
        }

        qDebug() << "Last convo message not yet seen";
        return {};
    }

    Q_ASSERT(lastReadMessage);
    const int currentUnreadCount = convo.getUnreadCount();

    if (lastReadMessage->getId() == lastConvoMessage.getId() && currentUnreadCount <= 0)
    {
        qDebug() << "Last read message already marked as read";
        return {};
    }

    // The last message in a convo can be a deleted message view. This delete view
    // does not show in the list of message itself (seems a bug in bsky to me).
    // If this delete view
    if (lastConvoMessage.isDeleted() && lastConvoMessage.getRev() > lastReadMessage->getRev())
    {
        qDebug() << "Last convo message is deleted and newer than last read";
        return lastConvoMessage.getId();
    }

    return lastReadMessage->getId();
}

void Chat::sendMessage(const QString& convoId, const QString& text,
                       const QString& replyToMessageId,
                       const QString& quoteUri, const QString& quoteCid,
                       const NamedLink::List& embeddedLinks)
{
    qDebug() << "Send message:" << text;

    if (!chatMaster())
        return;

    emit sendMessageProgress();

    const auto embeddedFacets = NamedLink::toFacetList(embeddedLinks);
    chatMaster()->createMessage(text, embeddedFacets,
        [this, presence=*mPresence, convoId, replyToMessageId, quoteUri, quoteCid](auto message){
            if (!presence)
                return;

            if (!replyToMessageId.isEmpty())
                chatMaster()->addReplyToRefToMessage(*message, replyToMessageId);

            continueSendMessage(convoId, message, quoteUri, quoteCid);
        });
}

void Chat::continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message, const QString& quoteUri, const QString& quoteCid)
{
    if (quoteUri.isEmpty())
    {
        continueSendMessage(convoId, message);
        return;
    }

    if (isJoinLinkUri(quoteUri))
    {
        if (!chatMaster())
            return;

        const QString code = getJoinLinkCodeFromUri(quoteUri);
        chatMaster()->addJoinLinkCodeToMessage(*message, code);
        continueSendMessage(convoId, message);
        return;
    }

    if (!postMaster())
        return;

    postMaster()->checkRecordExists(quoteUri, quoteCid,
        [this, presence=*mPresence, convoId, message, quoteUri, quoteCid]{
            if (!presence)
                return;

            if (chatMaster())
            {
                chatMaster()->addQuoteToMessage(*message, quoteUri, quoteCid);
                continueSendMessage(convoId, message);
            }
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Record not found:" << error << " - " << msg;
            emit sendMessageFailed(tr("Quoted record") + ": " + msg);
        });
}

void Chat::continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message)
{
    if (!mBsky)
        return;

    qDebug() << "Send message:" << message->toJson();
    mBsky->sendMessage(convoId, *message,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::MessageView::SharedPtr messageView){
            if (!presence)
                return;

            qDebug() << "Message sent:" << messageView->mId;
            emit sendMessageOk();
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Record not found:" << error << " - " << msg;
            emit sendMessageFailed(msg);
        });
}

void Chat::deleteMessage(const QString& convoId, const QString& messageId)
{
    qDebug() << "Delete message, convoId:" << convoId << "messageId:" << messageId;

    if (!mBsky)
        return;

    mBsky->deleteMessageForSelf(convoId, messageId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::DeletedMessageView::SharedPtr deletedView){
            if (!presence)
                return;

            qDebug() << "Message deleted:" << deletedView->mId;
            emit deleteMessageOk();
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "deleteMessage failed:" << error << " - " << msg;
            emit deleteMessageFailed(msg);
        });
}

void Chat::addReaction(const QString& convoId, const QString& messageId, const QString& emoji)
{
    qDebug() << "Add reaction, convoId:" << convoId << "messageId:" << messageId << "emoji:" << emoji;

    if (!mBsky)
        return;

    mBsky->addReaction(convoId, messageId, emoji,
        [this, presence=*mPresence, convoId](auto messageOutput){
            if (!presence)
                return;

            qDebug() << "Reaction added";
            const MessageView msg(*messageOutput->mMessage);
            auto* model = getMessageListModel(convoId);

            if (model)
                model->updateMessage(msg);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addReaction failed:" << error << " - " << msg;
            emit failure(msg);
        });
}

void Chat::deleteReaction(const QString& convoId, const QString& messageId, const QString& emoji)
{
    qDebug() << "Delete reaction, convoId:" << convoId << "messageId:" << messageId << "emoji:" << emoji;

    if (!mBsky)
        return;

    mBsky->removeReaction(convoId, messageId, emoji,
        [this, presence=*mPresence, convoId](auto messageOutput){
            if (!presence)
                return;

            qDebug() << "Reaction deleted";
            const MessageView msg(*messageOutput->mMessage);
            auto* model = getMessageListModel(convoId);

            if (model)
                model->updateMessage(msg);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addReaction failed:" << error << " - " << msg;
            emit failure(msg);
        });
}

void Chat::startMessagesUpdateTimer()
{
    if (!mMessagesUpdateTimer.isActive())
    {
        qDebug() << "Start messages update timer";
        mMessagesUpdateTimer.start(MESSAGES_UPDATE_INTERVAL);
    }
}

void Chat::stopMessagesUpdateTimer()
{
    qDebug() << "Stop messages update timer";
    mMessagesUpdateTimer.stop();
}

void Chat::setMessagesUpdating(const QString& convoId, bool updating)
{
    if (updating)
        mConvoIdUpdatingMessages.insert(convoId);
    else
        mConvoIdUpdatingMessages.erase(convoId);
}

void Chat::startConvosUnreadUpdateTimer()
{
    qDebug() << "Start convus unread update timer";
    mConvosUnreadUpdateTimer.start(CONVOS_UPDATE_INTERVAL);
}

void Chat::stopConvosUnreadUpdateTimer()
{
    qDebug() << "Stop convus unread update timer";
    mConvosUnreadUpdateTimer.stop();
}

void Chat::pause()
{
    qDebug() << "Pause";
    stopMessagesUpdateTimer();
    stopConvosUnreadUpdateTimer();
}
void Chat::resume()
{
    qDebug() << "Resume";

    if (!mMessageListModels.empty())
        startMessagesUpdateTimer();

    startConvosUnreadUpdateTimer();
    getConvosUnreadCounts();
}

}
