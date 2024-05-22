// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto MESSAGES_UPDATE_INTERVAL = 11s;
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
    mConvoListModel.clear();
    mMessageListModels.clear();
    mConvoIdUpdatingMessages.clear();
    setUnreadCount(0);
    setConvosInProgress(false);
    mLoaded = false;
    stopMessagesUpdateTimer();
    mChatMaster = nullptr;
    mPostMaster = nullptr;
    mPresence = std::make_unique<Presence>();
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
                mUnreadCount = 0;
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
            emit getConvosFailed(msg);
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
            mUnreadCount = 0;
            mConvoListModel.addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(*output);
            mLoaded = true;
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateConvos FAILED:" << error << " - " << msg;
        }
        );
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
        unread += convo->mUnreadCount;

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

void Chat::updateRead(const QString& convoId, const QString& messageId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update read convo:" << convoId;

    const auto* convo = mConvoListModel.getConvo(convoId);

    if (!convo)
        return;

    const int oldUnreadCount = convo->getUnreadCount();
    const MessageView& lastMessage = convo->getLastMessage();

    if (lastMessage.getId() == messageId && oldUnreadCount <= 0)
        return;

    mBsky->updateRead(convoId, Utils::makeOptionalString(messageId),
        [this, presence=*mPresence, oldUnreadCount](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            if (!presence)
                return;

            mConvoListModel.updateConvo(*output->mConvo);
            const int readCount = oldUnreadCount - output->mConvo->mUnreadCount;
            setUnreadCount(mUnreadCount - readCount);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateRead FAILED:" << error << " - " << msg;
        }
    );
}

void Chat::sendMessage(const QString& convoId, const QString& text, const QString& quoteUri, const QString& quoteCid)
{
    qDebug() << "Send message:" << text;

    if (!chatMaster())
        return;

    emit sendMessageProgress(tr("Sending message"));

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

    mConvosUpdateTimer.start();
}

}
