// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "verification_utils.h"

namespace Skywalker {

using namespace std::chrono_literals;

VerificationUtils::VerificationUtils(Constellation& constellation, QObject* parent) :
    WrappedSkywalker(parent),
    mConstellation(constellation),
    mIsVerifiedCache(24h, 500),
    mVerificationCache(10)
{
    // TODO:
    // This is the DID for Eurosky for testing.
    setVerifiers({"did:plc:ooensn4mr5mhznzypvxelfa3"});
}

void VerificationUtils::setVerifiers(const std::vector<QString>& verifierDids)
{
    Q_ASSERT(verifierDids.size() <= MAX_VERIFIERS);

    if (verifierDids.size() > MAX_VERIFIERS)
    {
        qWarning() << "Too many verifiers:" << verifierDids.size() << "max:" << MAX_VERIFIERS;
        return;
    }

    mVerifierDids = verifierDids;
    mVerifierDidIndex.clear();
    mVerifierDidIndex.insert(verifierDids.begin(), verifierDids.end());

    mIsVerifiedCache.clear();
    mVerificationCache.clear();
}

void VerificationUtils::isVerified(const BasicProfile& profile)
{
    if (profile.isNull())
        return;

    qDebug() << "Is verified:" << profile.getDid();
    const QString& did = profile.getDid();

    if (profile.getVerificationState().getVerifiedStatus() == QEnums::VERIFIED_STATUS_VALID)
    {
        emit verified(did, true);
        return;
    }

    if (mVerifierDids.empty())
        return;

    const bool* v = mIsVerifiedCache.object(did);

    if (v)
    {
        emit verified(did, *v);
        return;
    }

    // Inserting in the cache now, prevents multiple lookups for the same DID.
    // The cache will be updated when the verification status comes back from Constellation.
    mIsVerifiedCache.insert(did, new bool(false));

    mConstellation.hasBackLinks(did, "app.bsky.graph.verification:subject", mVerifierDids,
        [this, did](bool hasLinks){
            emit verified(did, hasLinks);
            mIsVerifiedCache.insert(did, new bool(hasLinks));
        },
        [this, did](const QString& error, const QString& message){
            qWarning() << "isVerified failed:" << error << "-" << message;
            mIsVerifiedCache.remove(did);
        });
}

void VerificationUtils::getVerifications(const BasicProfile& profile)
{
    if (profile.isNull())
        return;

    qDebug() << "Get verifications:" << profile.getDid();
    const QString& did = profile.getDid();

    if (mIsVerifiedCache.contains(did))
    {
        if (!*mIsVerifiedCache[did])
            return;
    }

    if (mVerifierDids.empty())
        return;

    if (mVerificationCache.contains(did))
    {
        auto* verificationList = mVerificationCache[did];
        emit verifications(did, *verificationList);
        return;
    }


    mVerificationCache.insert(did, new VerificationView::List);

    mConstellation.getBackLinks(did, "app.bsky.graph.verification:subject", mVerifierDids, {},
        [this, profile](Constellation::Backlinks::SharedPtr backlinks){
            const QString& did = profile.getDid();
            const bool hasRecords = !backlinks->mRecords.empty();
            mIsVerifiedCache.insert(did, new bool(hasRecords));

            if (!hasRecords)
                return;

            auto* verificationList = new VerificationView::List;
            verificationList->reserve(backlinks->mRecords.size());

            for (const auto& record : backlinks->mRecords)
            {
                const ATProto::ATUri atUri(record->mDid, record->mCollection, record->mRkey);
                auto verificationView = std::make_shared<ATProto::AppBskyActor::VerificationView>();
                verificationView->mIssuer = record->mDid;
                verificationView->mUri = atUri.toString();
                verificationView->mIsValid = true;
                verificationList->push_back(VerificationView{verificationView});

                getVerificationRecord(profile, record->mDid, record->mCollection, record->mRkey);
            }

            emit verifications(did, *verificationList);
            mVerificationCache.insert(did, verificationList);
        },
        [this, did](const QString& error, const QString& message){
            qWarning() << "getVerifications failed:" << error << "-" << message;
            mVerificationCache.remove(did);
        });
}

bool VerificationUtils::isVerifier(const QString& did)
{
    return mVerifierDidIndex.contains(did);
}

void VerificationUtils::getVerificationRecord(const BasicProfile& user, const QString& issuerDid, const QString& collection, const QString& rkey)
{
    const QString& userDid = user.getDid();
    qDebug() << "Get verification record for:" << userDid << "issuer:" << issuerDid;

    bskyClient()->getRecord(issuerDid, collection, rkey, {},
        [this, user, issuerDid](ATProto::ComATProtoRepo::Record::SharedPtr record){
            const QString& userDid = user.getDid();
            VerificationView::List* verificationList = mVerificationCache.object(userDid);

            if (!verificationList)
            {
                qWarning() << "Verification list not in cache:" << userDid;
                return;
            }

            try {
                auto verification = ATProto::AppBskyGraph::Verification::fromJson(record->mValue);

                auto it = std::find_if(verificationList->begin(), verificationList->end(),
                                       [issuerDid](const VerificationView& view){
                                           return view.getIssuer() == issuerDid;
                                       });

                if (it == verificationList->end())
                {
                    qWarning() << "Cannot find verification from issuer:" << issuerDid;
                    return;
                }

                const auto& atProtoView = it->getAtProtoView();
                atProtoView->mCreatedAt = verification->mCreatedAt;
                atProtoView->mDisplayName = verification->mDisplayName;
                atProtoView->mHandle = verification->mHandle;
                atProtoView->mIsValid = verification->mDisplayName == user.getDisplayName() && verification->mHandle == user.getHandle();
                emit verifications(userDid, *verificationList);
            }
            catch (ATProto::InvalidJsonException& e) {
                qWarning() << e.msg();
            }
        },
        [this, userDid, issuerDid](const QString& error, const QString& message){
            qWarning() << "getVerificationRecord failed:" << error << "-" << message;

            if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
            {
                VerificationView::List* verificationList = mVerificationCache.object(userDid);

                if (verificationList)
                {
                    verificationList->removeIf(
                        [issuerDid](const VerificationView& view){
                            return view.getIssuer() == issuerDid;
                        });

                    emit verifications(userDid, *verificationList);
                }
            }
        });
}

}
