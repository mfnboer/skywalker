// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "constellation.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <QCache>
#include <QObject>

namespace Skywalker {

class VerificationUtils : public WrappedSkywalker
{
    Q_OBJECT

public:
    explicit VerificationUtils(Constellation& constellation, QObject* parent = nullptr);

    Q_INVOKABLE void isVerified(const BasicProfile& profile);
    Q_INVOKABLE void getVerifications(const BasicProfile& profile);
    Q_INVOKABLE bool isVerifier(const QString& did);

signals:
    void verified(QString did, bool isVerified);
    void verifications(QString did, VerificationView::List);

private:
    void getVerificationRecord(const QString& userDid, const QString& issuerDid, const QString& collection, const QString& rkey);

    Constellation& mConstellation;
    std::vector<QString> mVerifierDids;
    std::unordered_set<QString> mVerifierDidSet;
    QCache<QString, bool> mIsVerifiedCache;
    QCache<QString, VerificationView::List> mVerificationCache;
};

}
