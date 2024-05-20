// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat.h"
#include "utils.h"

namespace Skywalker {

Chat::Chat(ATProto::Client::Ptr& bsky, const QString& mUserDid, QObject* parent) :
    QObject(parent),
    mBsky(bsky),
    mConvoListModel(mUserDid, this)
{
}

void Chat::clear()
{
    mConvoListModel.clear();
    mUnreadCount = 0;
    mGetConvosInProgress = false;
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

void Chat::updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output)
{
    const bool oldCount = mUnreadCount;

    for (const auto& convo : output.mConvos)
        mUnreadCount += convo->mUnreadCount;

    if (oldCount != mUnreadCount)
        emit unreadCountChanged();
}

void Chat::setConvosInProgress(bool inProgress)
{
    if (inProgress != mGetConvosInProgress)
    {
        mGetConvosInProgress = inProgress;
        emit getConvosInProgressChanged();
    }
}

}
