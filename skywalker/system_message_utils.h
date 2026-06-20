// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "svg_image.h"
#include "message_view.h"
#include <QObject>

namespace Skywalker {

class SystemMessageUtils : public QObject, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SystemMessageUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getMessage(const MessageView& view);

signals:
    void message(SvgImage* image, QString text);

private:
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataAddMember::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataRemoveMember::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataMemberJoin::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataMemberLeave::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataLockConvo::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataUnlockConvo::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataLockConvoPermanently::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataEditGroup::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataCreateJoinLink::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataEditJoinLink::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataEnableJoinLink::SharedPtr msg);
    void resolve(ATProto::ChatBskyConvo::SystemMessageDataDisableJoinLink::SharedPtr msg);

    template <typename Msg>
    std::function<void()> tryResolveAgain(SvgImage* image, const Msg& msg);
};

}
