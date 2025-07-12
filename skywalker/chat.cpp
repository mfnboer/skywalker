// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto MESSAGES_UPDATE_INTERVAL = 9s;
static constexpr auto CONVOS_UPDATE_INTERVAL = 31s;
static constexpr char const* DM_ACCESS_ERROR = "Your APP password does not allow access to your direct messages. Create a new APP password that allows access.";

Chat::Chat(ATProto::Client::Ptr& bsky, const QString& userDid, QObject* parent) :
    QObject(parent),
    mPresence(std::make_unique<Presence>()),
    mBsky(bsky),
    mUserDid(userDid),
    mAcceptedConvoListModel(userDid, this),
    mRequestConvoListModel(userDid, this)
{
    connect(&mMessagesUpdateTimer, &QTimer::timeout, this, [this]{ updateMessages(); });
    connect(&mAcceptedConvosUpdateTimer, &QTimer::timeout, this, [this]{ updateConvos(QEnums::CONVO_STATUS_ACCEPTED); });
    connect(&mRequestConvosUpdateTimer, &QTimer::timeout, this, [this]{ updateConvos(QEnums::CONVO_STATUS_REQUEST); });
    connect(&mAcceptedConvoListModel, &ConvoListModel::unreadCountChanged, this, [this]{ updateTotalUnreadCount(); });
    connect(&mRequestConvoListModel, &ConvoListModel::unreadCountChanged, this, [this]{ updateTotalUnreadCount(); });
}

void Chat::reset()
{
    qDebug() << "Reset chat";
    stopMessagesUpdateTimer();
    stopConvosUpdateTimer(QEnums::CONVO_STATUS_ACCEPTED);
    stopConvosUpdateTimer(QEnums::CONVO_STATUS_REQUEST);

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
            qDebug() << "Allow incoming chat:" << mAllowIncomingChat;
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to get chat settings:" << error << "-" << msg;
            mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
        });
}

void Chat::updateSettings(QEnums::AllowIncomingChat allowIncoming)
{
    qDebug() << "Update settings";
    Q_ASSERT(mBsky);

    if (!chatMaster())
        return;

    ATProto::ChatBskyActor::Declaration declaration;
    declaration.mAllowIncoming = (ATProto::AppBskyActor::AllowIncomingType)allowIncoming;

    chatMaster()->updateDeclaration(mUserDid, declaration,
        [this, presence=*mPresence, allowIncoming]{
            if (!presence)
                return;

            mAllowIncomingChat = allowIncoming;
            qDebug() << "Allow incoming chat:" << mAllowIncomingChat;
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

void Chat::getAllConvos()
{
    // Spread requests in times
    getConvos(QEnums::CONVO_STATUS_ACCEPTED);
    QTimer::singleShot(1s, this, [this]{ getConvos(QEnums::CONVO_STATUS_REQUEST); });
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

    startConvosUpdateTimer(status);
    model->setGetConvosInProgress(true);
    mBsky->listConvos({}, false, ATProto::ChatBskyConvo::ConvoStatus(status), Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, status, cursor](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Get convos ok:" << status;
            auto* model = getConvoListModel(status);

            if (!model)
                return;

            if (cursor.isEmpty())
            {
                model->clear();
                setUnreadCount(status, 0);
            }

            model->addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(status, *output);
            model->setLoaded(true);
            model->setGetConvosInProgress(false);
        },
        [this, presence=*mPresence, status](const QString& error, const QString& msg){
            if (!presence)
                return;

            auto* model = getConvoListModel(status);

            if (!model)
                return;

            qDebug() << "getConvos FAILED:" << error << " - " << msg;
            model->setGetConvosInProgress(false);

            if (error == ATProto::ATProtoErrorMsg::INVALID_TOKEN)
            {
                stopConvosUpdateTimer(status);
                emit failure(DM_ACCESS_ERROR);
            }
            else
            {
                emit failure(msg);
            }
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

QString Chat::getLastRevIncludingReactions(ConvoListModel* model, ATProto::ChatBskyConvo::ConvoViewList& convos)
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

void Chat::updateConvos(QEnums::ConvoStatus status)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update convos:" << status;

    mBsky->listConvos({}, false, ATProto::ChatBskyConvo::ConvoStatus(status), {},
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

    if (mLeaveConvoInProgress)
    {
        qDebug() << "Leave convo still in progress";
        return;
    }

    setLeaveConvoInProgress(true);

    mBsky->leaveConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::LeaveConvoOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Left convo:" << output->mConvoId;
            setLeaveConvoInProgress(false);
            mAcceptedConvoListModel.deleteConvo(output->mConvoId);
            mRequestConvoListModel.deleteConvo(output->mConvoId);
            emit leaveConvoOk();
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "leaveConvo FAILED:" << error << " - " << msg;
            setLeaveConvoInProgress(false);
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

void Chat::updateUnreadCount(QEnums::ConvoStatus status, const ATProto::ChatBskyConvo::ConvoListOutput& output)
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

void Chat::setLeaveConvoInProgress(bool inProgress)
{
    if (inProgress != mLeaveConvoInProgress)
    {
        mLeaveConvoInProgress = inProgress;
        emit leaveConvoInProgressChanged();
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

void Chat::updateBlockingUri(const QString& did, const QString& blockingUri)
{
    mAcceptedConvoListModel.updateBlockingUri(did, blockingUri);
    mRequestConvoListModel.updateBlockingUri(did, blockingUri);
}

MessageListModel* Chat::getMessageListModel(const QString& convoId)
{
    auto& model = mMessageListModels[convoId];

    if (!model)
    {
        qDebug() << "Create message list model for convo:" << convoId;
        model = std::make_unique<MessageListModel>(mUserDid, this);
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

    const int oldUnreadCount = convo->getUnreadCount();

    mBsky->updateRead(convoId, lastReadMessageId,
        [this, presence=*mPresence, status, oldUnreadCount](ATProto::ChatBskyConvo::ConvoOutput::SharedPtr output){
            if (!presence)
                return;

            auto* model = getConvoListModel(status);

            if (!model)
                return;

            model->updateConvo(*output->mConvo);

            if (!output->mConvo->mMuted)
            {
                const int readCount = oldUnreadCount - output->mConvo->mUnreadCount;
                const int newUnreadCount = model->getUnreadCount() - readCount;
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
                       const QString& quoteUri, const QString& quoteCid,
                       const WebLink::List& embeddedLinks)
{
    qDebug() << "Send message:" << text;

    if (!chatMaster())
        return;

    emit sendMessageProgress();

    const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);
    chatMaster()->createMessage(text, embeddedFacets,
        [this, presence=*mPresence, convoId, quoteUri, quoteCid](auto message){
            if (!presence)
                return;

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

void Chat::startConvosUpdateTimer(QEnums::ConvoStatus status)
{
    qDebug() << "Start convos update timer:" << status;

    switch (status)
    {
    case QEnums::CONVO_STATUS_REQUEST:
        mRequestConvosUpdateTimer.start(CONVOS_UPDATE_INTERVAL);
        break;
    case QEnums::CONVO_STATUS_ACCEPTED:
        mAcceptedConvosUpdateTimer.start(CONVOS_UPDATE_INTERVAL);
        break;
    case QEnums::CONVO_STATUS_UNKNOWN:
        qWarning() << "Unknown status";
        break;
    }


}

void Chat::stopConvosUpdateTimer(QEnums::ConvoStatus status)
{
    qDebug() << "Stop convos update timer:" << status;

    switch (status)
    {
    case QEnums::CONVO_STATUS_REQUEST:
        mRequestConvosUpdateTimer.stop();
        break;
    case QEnums::CONVO_STATUS_ACCEPTED:
        mAcceptedConvosUpdateTimer.stop();
        break;
    case QEnums::CONVO_STATUS_UNKNOWN:
        qWarning() << "Unknown status";
        break;
    }
}

void Chat::pause()
{
    qDebug() << "Pause";
    stopMessagesUpdateTimer();
    stopConvosUpdateTimer(QEnums::CONVO_STATUS_ACCEPTED);
    stopConvosUpdateTimer(QEnums::CONVO_STATUS_REQUEST);
}
void Chat::resume()
{
    qDebug() << "Resume";

    if (!mMessageListModels.empty())
        startMessagesUpdateTimer();

    getConvos(QEnums::CONVO_STATUS_REQUEST);
    getConvos(QEnums::CONVO_STATUS_ACCEPTED);
}

}
