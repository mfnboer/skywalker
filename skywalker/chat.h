// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_list_model.h"
#include <atproto/lib/client.h>

namespace Skywalker {

class Chat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConvoListModel* convoListModel READ getConvoListModel CONSTANT FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount NOTIFY unreadCountChanged FINAL)
    Q_PROPERTY(bool getConvosInProgress READ isGetConvosInProgress NOTIFY getConvosInProgressChanged FINAL)

public:
    explicit Chat(ATProto::Client::Ptr& bsky, const QString& mUserDid, QObject* parent = nullptr);

    void clear();
    Q_INVOKABLE void getConvos(const QString& cursor = "");
    Q_INVOKABLE void getConvosNextPage();
    Q_INVOKABLE bool convosLoaded() const { return mLoaded; }

    ConvoListModel* getConvoListModel() { return &mConvoListModel; }
    int getUnreadCount() const { return mUnreadCount; }

    bool isGetConvosInProgress() const { return mGetConvosInProgress; }
    void setConvosInProgress(bool inProgress);

signals:
    void unreadCountChanged();
    void getConvosInProgressChanged();
    void getConvosFailed(QString error);

private:
    void updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output);

    ATProto::Client::Ptr& mBsky;
    ConvoListModel mConvoListModel;
    int mUnreadCount = 0;
    bool mGetConvosInProgress = false;
    bool mLoaded = false;
};

}
