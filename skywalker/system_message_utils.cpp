// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "system_message_utils.h"
#include "author_cache.h"
#include "svg_filled.h"
#include "svg_outline.h"

namespace Skywalker {

SystemMessageUtils::SystemMessageUtils(QObject* parent) :
    QObject(parent)
{
}

void SystemMessageUtils::getMessage(const MessageView& view)
{
    Q_ASSERT(view.isSystemMessage());

    if (!view.isSystemMessage())
    {
        qWarning() << "Not as system message:" << view.getId() << "text:" << view.getText();
        return;
    }

    const auto& systemMessage = view.getSystemMessage();
    Q_ASSERT(systemMessage);

    std::visit([this](auto&& msg){
            if (msg)
                resolve(msg);
        }, systemMessage->mData);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataAddMember::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sPersonAdd;
    auto& cache = AuthorCache::instance();
    const BasicProfile* member = cache.get(msg->mMember->mDid);

    if (!member)
    {
        cache.putProfile(msg->mMember->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const BasicProfile* addedBy = cache.get(msg->mAddedBy->mDid);

    if (!addedBy)
    {
        cache.putProfile(msg->mAddedBy->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("%1 added by %2").arg(member->getName(), addedBy->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataRemoveMember::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sPersonRemove;
    auto& cache = AuthorCache::instance();
    const BasicProfile* member = cache.get(msg->mMember->mDid);

    if (!member)
    {
        cache.putProfile(msg->mMember->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const BasicProfile* removedBy = cache.get(msg->mRemovedBy->mDid);

    if (!removedBy)
    {
        cache.putProfile(msg->mRemovedBy->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("%1 removed by %2").arg(member->getName(), removedBy->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataMemberJoin::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sLogin;
    auto& cache = AuthorCache::instance();
    const BasicProfile* member = cache.get(msg->mMember->mDid);

    if (!member)
    {
        cache.putProfile(msg->mMember->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("%1 joined").arg(member->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataMemberLeave::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sLogout;
    auto& cache = AuthorCache::instance();
    const BasicProfile* member = cache.get(msg->mMember->mDid);

    if (!member)
    {
        cache.putProfile(msg->mMember->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("%1 left").arg(member->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataLockConvo::SharedPtr msg)
{
    auto* icon = SvgFilled::instance()->sLock;
    auto& cache = AuthorCache::instance();
    const BasicProfile* lockedBy = cache.get(msg->mLockedBy->mDid);

    if (!lockedBy)
    {
        cache.putProfile(msg->mLockedBy->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("Group is locked by %1").arg(lockedBy->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataUnlockConvo::SharedPtr msg)
{
    auto* icon = SvgFilled::instance()->sLockOpenRight;
    auto& cache = AuthorCache::instance();
    const BasicProfile* unlockedBy = cache.get(msg->mUnlockedBy->mDid);

    if (!unlockedBy)
    {
        cache.putProfile(msg->mUnlockedBy->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("Group is unlocked by %1").arg(unlockedBy->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataLockConvoPermanently::SharedPtr msg)
{
    auto* icon = SvgFilled::instance()->sLock;
    auto& cache = AuthorCache::instance();
    const BasicProfile* lockedBy = cache.get(msg->mLockedBy->mDid);

    if (!lockedBy)
    {
        cache.putProfile(msg->mLockedBy->mDid, tryResolveAgain(icon, msg));
        return;
    }

    const QString text = tr("Group is locked permanently by %1").arg(lockedBy->getName());
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataEditGroup::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sEditNote;
    const QString text = tr("Group name changed from '%1' to '%2'")
                             .arg(msg->mOldName.value_or(""), msg->mNewName.value_or(""));
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataCreateJoinLink::SharedPtr)
{
    auto* icon = SvgOutline::instance()->sAddLink;
    const QString text = tr("Join link created");
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataEditJoinLink::SharedPtr)
{
    auto* icon = SvgOutline::instance()->sLink;
    const QString text = tr("Join link changed");
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataEnableJoinLink::SharedPtr)
{
    auto* icon = SvgOutline::instance()->sLink;
    const QString text = tr("Join link enabled");
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::ChatBskyConvo::SystemMessageDataDisableJoinLink::SharedPtr)
{
    auto* icon = SvgOutline::instance()->sLinkOff;
    const QString text = tr("Join link disabled");
    emit message(icon, text);
}

void SystemMessageUtils::resolve(ATProto::UnknownVariant::SharedPtr msg)
{
    auto* icon = SvgOutline::instance()->sWarning;
    const QString text = tr("Unknown message: %1").arg(msg->mType);
    emit message(icon, text);
}

template <typename Msg>
std::function<void()> SystemMessageUtils::tryResolveAgain(SvgImage* image, const Msg& msg)
{
    emit message(image, "");

    return
        [this, presence=getPresence(), msg]{
            if (!presence)
                return;

            resolve(msg);
        };
}

}
