// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "invite_code_store.h"
#include "author_cache.h"

namespace Skywalker {

InviteCodeStore::InviteCodeStore(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void InviteCodeStore::clear()
{
    mAvailableCount = 0;
    mFailedToLoad = false;
    mCodes.clear();
    mUsedSincePreviousSignIn.clear();
}

void InviteCodeStore::load(QDateTime prevSignIn)
{
    qDebug() << "Load invite codes, previous sign in:" << prevSignIn;
    clear();

    bskyClient()->getAccountInviteCodes(
        [this, prevSignIn](auto output){
            auto compCreated = [](InviteCode* lhs, InviteCode* rhs){ return lhs->getCreatedAt() > rhs->getCreatedAt(); };
            std::set<InviteCode*, decltype(compCreated)> availableCodes(compCreated);
            auto compUsed = [](InviteCode* lhs, InviteCode* rhs){ return lhs->getUsedAt() > rhs->getUsedAt(); };
            std::set<InviteCode*, decltype(compUsed)> usedCodes(compUsed);

            for (const auto& code : output->mCodes)
            {
                const auto inviteCode = new InviteCode(*code, this);

                if (inviteCode->isAvailable())
                    availableCodes.insert(inviteCode);
                else
                    usedCodes.insert(inviteCode);
            }

            mAvailableCount = (int)availableCodes.size();

            for (auto* code : availableCodes)
                mCodes.append(code);

            for (auto* code : usedCodes)
            {
                mCodes.append(code);

                if (prevSignIn.isValid() && code->getUsedAt() > prevSignIn)
                {
                    qInfo() << "New invite code usage:" << code->getCode() << code->getUsedAt() << code->getUsedByDid();
                    mUsedSincePreviousSignIn.append(code);
                }
            }

            emit loaded();
        },
        [this](const QString& error){
            qDebug() << "Load invite codes failed:" << error;
            mFailedToLoad = true;
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
            AuthorCache::instance().put(code.getUsedBy());
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
