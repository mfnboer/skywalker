// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

Chat::Chat(ATProto::Client::Ptr& bsky, const QString& userDid, QObject* parent) :
    QObject(parent),
    mBsky(bsky),
    mUserDid(userDid),
    mConvoListModel(userDid, this)
{
}

void Chat::clear()
{
    mConvoListModel.clear();
    setUnreadCount(0);
    setConvosInProgress(false);
    mLoaded = false;
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
        model = std::make_unique<MessageListModel>(mUserDid, this);

    return model.get();
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

            if (cursor.isEmpty())
                model->clear();

            model->addMessages(output->mMessages, output->mCursor.value_or(""));
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
    const QString& cursor = model->getCursor();

    if(cursor.isEmpty())
    {
        qDebug() << "Last page reached";
        return;
    }

    getMessages(convoId, cursor);
}

void Chat::updateRead(const QString& convoId, int readCount)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update read convo:" << convoId << "read:" << readCount;

    if (readCount <= 0)
        return;

    mBsky->updateRead(convoId, {},
        [this, readCount](ATProto::ChatBskyConvo::ConvoOuput::Ptr output){
            mConvoListModel.updateConvo(*output->mConvo);
            setUnreadCount(mUnreadCount - readCount);
        },
        [](const QString& error, const QString& msg){
            qDebug() << "updateRead FAILED:" << error << " - " << msg;
        }
    );
}

}
