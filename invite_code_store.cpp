// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "invite_code_store.h"

namespace Skywalker {

InviteCodeStore::InviteCodeStore(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void InviteCodeStore::load()
{
    bskyClient()->getAccountInviteCodes(
        [this](auto output){
            mCodes.clear();
            mAvailableCount = 0;

            for (const auto& code : output->mCodes)
            {
                const auto inviteCode = new InviteCode(*code, this);
                mCodes.prepend(inviteCode);

                if (inviteCode->isAvailable())
                    ++mAvailableCount;
            }

            emit loaded();
        },
        [this](const QString& error){
            qDebug() << "load invite codes:" << error;
            getSkywalker()->statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

const QVariantList InviteCodeStore::getCodes() const
{
    QVariantList list;

    for (auto* code : mCodes)
        list.append(QVariant::fromValue(code));

    return list;
}

void InviteCodeStore::retrieveUsedByProfile(InviteCode& code)
{
    code.setRetrievingUsedByProfile(true);

    bskyClient()->getProfile(code.getUsedByDid(),
        [&code](auto profile){
            code.setUsedBy(BasicProfile(profile.get()));
            code.setRetrievingUsedByProfile(false);
        },
        [this, &code](const QString& error){
            qDebug() << "Cannot retrieve usedBy profile:" << error;
            code.setRetrievingUsedByProfile(false);
            getSkywalker()->statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

void InviteCodeStore::codeCopied()
{
    mSkywalker->showStatusMessage(tr("Invite code copied to clipboard"), QEnums::STATUS_LEVEL_INFO);
}

}
