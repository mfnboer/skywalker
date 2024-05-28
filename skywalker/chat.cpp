// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto MESSAGES_UPDATE_INTERVAL = 9s;
static constexpr auto CONVOS_UPDATE_INTERVAL = 31s;

Chat::Chat(ATProto::Client::Ptr& bsky, const QString& userDid, QObject* parent) :
    QObject(parent),
    mPresence(std::make_unique<Presence>()),
    mBsky(bsky),
    mUserDid(userDid),
    mConvoListModel(userDid, this)
{
    connect(&mMessagesUpdateTimer, &QTimer::timeout, this, [this]{ updateMessages(); });
    connect(&mConvosUpdateTimer, &QTimer::timeout, this, [this]{ updateConvos(); });
}

void Chat::reset()
{
    qDebug() << "Reset chat";
    stopMessagesUpdateTimer();
    stopConvosUpdateTimer();
    mConvoListModel.clear();
    mMessageListModels.clear();
    mConvoIdUpdatingMessages.clear();
    setUnreadCount(0);
    setConvosInProgress(false);
    setStartConvoInProgress(false);
    setMessagesInProgress(false);
    mLoaded = false;
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
        [this, presence=*mPresence](ATProto::ChatBskyActor::Declaration::Ptr declaration){
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

            if (error != ATProto::ATProtoErrorMsg::INVALID_REQUEST)
                emit settingsFailed(msg);
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
    return mConvoListModel.getLastRev();
}

void Chat::getConvos(const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get convos:" << cursor;

    if (mGetConvosInProgress)
    {
        qDebug() << "Get convos still in progress";
        return;
    }

    setConvosInProgress(true);
    mBsky->listConvos({}, Utils::makeOptionalString(cursor),
        [this, presence=*mPresence, cursor](ATProto::ChatBskyConvo::ConvoListOutput::Ptr output){
            if (!presence)
                return;

            if (cursor.isEmpty())
            {
                mConvoListModel.clear();
                setUnreadCount(0);
            }

            mConvoListModel.addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(*output);
            mLoaded = true;
            startConvosUpdateTimer();
            setConvosInProgress(false);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getConvos FAILED:" << error << " - " << msg;
            setConvosInProgress(false);
            emit failure(msg);
        }
    );
}

void Chat::getConvosNextPage()
{
    const QString& cursor = mConvoListModel.getCursor();
    if(cursor.isEmpty())
    {
        qDebug() << "Last page reached";
        return;
    }

    getConvos(cursor);
}

void Chat::updateConvos()
{
    Q_ASSERT(mBsky);
    qDebug() << "Update convos";

    mBsky->listConvos({}, {},
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoListOutput::Ptr output){
            if (!presence)
                return;

            if (output->mConvos.empty())
            {
                qDebug() << "No convos";
                return;
            }

            const QString rev = output->mConvos.front()->mRev;

            if (rev == mConvoListModel.getLastRev())
            {
                qDebug() << "No updated convos, rev:" << rev;
                return;
            }

            mConvoListModel.clear();
            setUnreadCount(0);
            mConvoListModel.addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(*output);
            mLoaded = true;
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateConvos FAILED:" << error << " - " << msg;
        }
        );
}

void Chat::startConvoForMembers(const QStringList& dids)
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
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            if (!presence)
                return;

            setStartConvoInProgress(false);
            const ConvoView convo(*output->mConvo, mUserDid);
            emit startConvoForMembersOk(convo);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            setStartConvoInProgress(false);
            qDebug() << "startConvoForMembers FAILED:" << error << " - " << msg;
            emit startConvoForMembersFailed(msg);
        });
}

void Chat::startConvoForMember(const QString& did)
{
    QStringList dids(did);
    startConvoForMembers(dids);
}

void Chat::leaveConvo(const QString& convoId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Leave convo:" << convoId;

    mBsky->leaveConvo(convoId,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::LeaveConvoOutput::Ptr output){
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
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            if (!presence)
                return;

            mConvoListModel.updateConvo(*output->mConvo);
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
        [this, presence=*mPresence](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            if (!presence)
                return;

            mConvoListModel.updateConvo(*output->mConvo);
        },
        [this, presence=*mPresence](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "unmuteConvo FAILED:" << error << " - " << msg;
            emit failure(msg);
        });
}

void Chat::setUnreadCount(int unread)
{
    if (unread < 0)
    {
        qWarning() << "Negative unread:" << unread;
        unread = 0;
    }

    if (mUnreadCount != unread)
    {
        mUnreadCount = unread;
        emit unreadCountChanged();
    }
}

void Chat::updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output)
{
    int unread = mUnreadCount;

    for (const auto& convo : output.mConvos)
    {
        if (!convo->mMuted)
            unread += convo->mUnreadCount;
    }

    setUnreadCount(unread);
}

void Chat::setConvosInProgress(bool inProgress)
{
    if (inProgress != mGetConvosInProgress)
    {
        mGetConvosInProgress = inProgress;
        emit getConvosInProgressChanged();
    }
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
        [this, presence=*mPresence, convoId, cursor](ATProto::ChatBskyConvo::GetMessagesOutput::Ptr output){
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
        [this, presence=*mPresence, convoId](ATProto::ChatBskyConvo::GetMessagesOutput::Ptr output){
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

    const auto* convo = mConvoListModel.getConvo(convoId);

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
        [this, presence=*mPresence, oldUnreadCount](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            if (!presence)
                return;

            mConvoListModel.updateConvo(*output->mConvo);

            if (!output->mConvo->mMuted)
            {
                const int readCount = oldUnreadCount - output->mConvo->mUnreadCount;
                setUnreadCount(mUnreadCount - readCount);
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

    mBsky->sendMessage(convoId, *message,
        [this, presence=*mPresence](ATProto::ChatBskyConvo::MessageView::Ptr messageView){
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
        [this, presence=*mPresence](ATProto::ChatBskyConvo::DeletedMessageView::Ptr deletedView){
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

void Chat::startConvosUpdateTimer()
{
    qDebug() << "Start convos update timer";
    mConvosUpdateTimer.start(CONVOS_UPDATE_INTERVAL);
}

void Chat::stopConvosUpdateTimer()
{
    qDebug() << "Stop convos update timer";
    mConvosUpdateTimer.stop();
}

void Chat::pause()
{
    qDebug() << "Pause";
    stopMessagesUpdateTimer();
    stopConvosUpdateTimer();
}
void Chat::resume()
{
    qDebug() << "Resume";

    if (!mMessageListModels.empty())
        startMessagesUpdateTimer();

    if (mLoaded)
    {
        getConvos();
        startConvosUpdateTimer();
    }
}

}
