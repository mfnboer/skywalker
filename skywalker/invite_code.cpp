// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "invite_code.h"
#include "author_cache.h"
#include "invite_code_store.h"
#include <QClipboard>
#include <QGuiApplication>

namespace Skywalker {

InviteCode::InviteCode(const ATProto::ComATProtoServer::InviteCode& code, InviteCodeStore* store) :
    QObject(store),
    mStore(store),
    mCode(code.mCode),
    mAvailable(code.mAvailable > (int)code.mUses.size()),
    mDisabled(code.mDisabled),
    mCreatedAt(code.mCreatedAt)
{
    Q_ASSERT(mStore);

    if (!code.mUses.empty())
    {
        mUsedByDid = code.mUses.back()->mUsedBy;
        mUsedAt = code.mUses.back()->mUsedAt;
    }
}

void InviteCode::copyToClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(mCode);

    if (mStore)
        mStore->codeCopied();
}

const BasicProfile& InviteCode::getUsedBy()
{
    if (!isUsed())
        return mUsedBy;

    if (!mUsedBy.isNull())
        return mUsedBy;

    const BasicProfile* profile = AuthorCache::instance().get(mUsedByDid);

    if (profile)
    {
        mUsedBy = *profile;
        return mUsedBy;
    }

    if (mStore && !mRetrievingUsedByProfile)
    {
        mStore->retrieveUsedByProfile(*this);
    }

    return mUsedBy;
}

void InviteCode::setUsedBy(const BasicProfile& profile)
{
    mUsedBy = profile;
    emit usedByChanged();
}

}
