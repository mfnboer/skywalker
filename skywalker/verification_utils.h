// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "constellation.h"
#include "expiry_cache.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <QCache>
#include <QObject>

namespace Skywalker {

class VerificationUtils : public WrappedSkywalker
{
    Q_OBJECT

public:
    static constexpr int MAX_VERIFIERS = 10;

    explicit VerificationUtils(Constellation& constellation, QObject* parent = nullptr);

    void setVerifiers(const std::vector<QString>& verifierDids);
    Q_INVOKABLE void isVerified(const BasicProfile& profile);
    Q_INVOKABLE void getVerifications(const BasicProfile& profile);
    Q_INVOKABLE bool isVerifier(const QString& did);

signals:
    void verified(QString did, bool isVerified);
    void verifications(QString did, VerificationView::List);

private:
    void getVerificationRecord(const BasicProfile& user, const QString& issuerDid, const QString& collection, const QString& rkey);

    Constellation& mConstellation;
    std::vector<QString> mVerifierDids;
    std::unordered_set<QString> mVerifierDidIndex;
    ExpiryCache<QString, bool> mIsVerifiedCache;
    QCache<QString, VerificationView::List> mVerificationCache;
};

}
