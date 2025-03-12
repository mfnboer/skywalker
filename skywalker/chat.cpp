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
    mRequestConvoListModel(mUserDid, this)
{
    connect(&mMessagesUpdateTimer, &QTimer::timeout, this, [this]{ updateMessages(); });
    connect(&mAcceptedConvosUpdateTimer, &QTimer::timeout, this, [this]{ updateConvos(QEnums::CONVO_STATUS_ACCEPTED); });
    connect(&mRequestConvosUpdateTimer, &QTimer::timeout, this, [this]{ updateConvos(QEnums::CONVO_STATUS_REQUEST); });
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
    qDebug() << "Init settings";
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

            qWarning() << "Failed to get chat settings:" << error << "-" << msg;
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
    const auto lastRevAccepted = mAcceptedConvoListModel.getLastRev();
    const auto lastRevRequested = mRequestConvoListModel.getLastRev();

    return std::max(lastRevAccepted, lastRevRequested);
}

void Chat::getConvos(QEnums::ConvoStatus status, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get convos:" << status << "cursor:" << cursor;
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    if (model->isGetConvosInProgress())
    {
        qDebug() << "Get convos still in progress:" << status;
        return;
    }

    model->setGetConvosInProgress(true);
    mBsky->listConvos({}, false, ATProto::ChatBskyConvo::ConvoStatus(status), Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, status, cursor](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            if (!presence)
                return;

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
            startConvosUpdateTimer(status);
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

void Chat::updateConvos(QEnums::ConvoStatus status)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update convos:" << status;

    mBsky->listConvos({}, true, ATProto::ChatBskyConvo::ConvoStatus(status), {},
        [this, presence=*mPresence, status](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            if (!presence)
                return;

            if (output->mConvos.empty())
            {
                qDebug() << "No convos";
                return;
            }

            auto* model = getConvoListModel(status);

            if (!model)
                return;

            const QString rev = output->mConvos.front()->mRev;

            if (rev == model->getLastRev())
            {
                qDebug() << "No updated convos, rev:" << rev;
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
            const ConvoView convo(*output->mConvo, mUserDid);
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

void Chat::leaveConvo(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Leave convo:" << convoId;

    mBsky->leaveConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::LeaveConvoOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Left convo:" << output->mConvoId;
            emit leaveConvoOk();
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "leaveConvo FAILED:" << error << " - " << msg;
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

void Chat::setUnreadCount(QEnums::ConvoStatus status, int unread)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    model->setUnreadCount(unread);
    const int totalUnread = mAcceptedConvoListModel.getUnreadCount() + mRequestConvoListModel.getUnreadCount();

    if (mUnreadCount != totalUnread)
    {
        mUnreadCount = totalUnread;
        emit unreadCountChanged();
    }
}

void Chat::updateUnreadCount(QEnums::ConvoStatus status, const ATProto::ChatBskyConvo::ConvoListOutput& output)
{
    auto* model = getConvoListModel(status);

    if (!model)
        return;

    model->updateUnreadCount(output);
    setUnreadCount(status, model->getUnreadCount());
}

void Chat::setStartConvoInProgress(bool inProgress)
{
    if (inProgress != mStartConvoInProgress)
    {
        mStartConvoInProgress = inProgress;
        emit startConvoInProgressChanged();
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

void Chat::sendMessage(const QString& convoId, const QString& text, const QString& quoteUri, const QString& quoteCid)
{
    qDebug() << "Send message:" << text;

    if (!chatMaster())
        return;

    emit sendMessageProgress();

    chatMaster()->createMessage(text,
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

    if (mAcceptedConvoListModel.isLoaded())
    {
        getConvos(QEnums::CONVO_STATUS_ACCEPTED);
        startConvosUpdateTimer(QEnums::CONVO_STATUS_ACCEPTED);
    }

    if (mRequestConvoListModel.isLoaded())
    {
        getConvos(QEnums::CONVO_STATUS_REQUEST);
        startConvosUpdateTimer(QEnums::CONVO_STATUS_REQUEST);
    }
}

}
