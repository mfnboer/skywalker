// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto MESSAGES_UPDATE_INTERVAL = 11s;

Chat::Chat(ATProto::Client::Ptr& bsky, const QString& userDid, QObject* parent) :
    QObject(parent),
    mBsky(bsky),
    mUserDid(userDid),
    mConvoListModel(userDid, this)
{
    connect(&mMessagesUpdateTimer, &QTimer::timeout, this, [this]{ updateMessages(); });
}

void Chat::clear()
{
    mConvoListModel.clear();
    mMessageListModels.clear();
    mConvoIdUpdatingMessages.clear();
    setUnreadCount(0);
    setConvosInProgress(false);
    mLoaded = false;
    stopMessagesUpdateTimer();
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
        [this, cursor](ATProto::ChatBskyConvo::ConvoListOutput::Ptr output){
            if (cursor.isEmpty())
            {
                mConvoListModel.clear();
                mUnreadCount = 0;
            }

            mConvoListModel.addConvos(output->mConvos, output->mCursor.value_or(""));
            updateUnreadCount(*output);
            mLoaded = true;
            setConvosInProgress(false);
        },
        [this](const QString& error, const QString& msg){
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
        [this, convoId, cursor](ATProto::ChatBskyConvo::GetMessagesOutput::Ptr output){
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
        [this](const QString& error, const QString& msg){
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
        [this, convoId](ATProto::ChatBskyConvo::GetMessagesOutput::Ptr output){
            setMessagesUpdating(convoId, false);
            auto* model = getMessageListModel(convoId);

            if (!model)
            {
                qDebug() << "Model already closed for convo:" << convoId;
                return;
            }

            model->updateMessages(output->mMessages, output->mCursor.value_or(""));
        },
        [this, convoId](const QString& error, const QString& msg){
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
        return;

    const int oldUnreadCount = convo->getUnreadCount();

    if (oldUnreadCount <= 0)
        return;

    const MessageView& lastMsg = convo->getLastMessage();

    mBsky->updateRead(convoId, Utils::makeOptionalString(lastMsg.getId()),
        [this, oldUnreadCount](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            mConvoListModel.updateConvo(*output->mConvo);
            const int readCount = oldUnreadCount - output->mConvo->mUnreadCount;
            setUnreadCount(mUnreadCount - readCount);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateRead FAILED:" << error << " - " << msg;
        }
    );
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

}
