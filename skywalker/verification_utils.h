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
    Q_PROPERTY(int MAX_VERIFIERS MEMBER MAX_VERIFIERS CONSTANT)

public:
    static constexpr int MAX_VERIFIERS = 10;

    explicit VerificationUtils(Constellation& constellation, QObject* parent = nullptr);

    void addVerifier(const QString& did, const QString& listItemUri);
    void removeVerifier(const QString& did);
    const QString* getListItemUri(const QString& did) const;

    void setListUri(const QString& uri) { mListUri = uri; }
    Q_INVOKABLE QString getListUri() const { return mListUri; }

    int size() const { return mVerifierDids.size(); }

    Q_INVOKABLE void isVerified(const BasicProfile& profile);
    Q_INVOKABLE void getVerifications(const BasicProfile& profile);
    Q_INVOKABLE bool isVerifier(const QString& did);

    void saveCache();
    void loadCache();

signals:
    void verified(QString did, bool isVerified);
    void verifications(QString did, VerificationView::List);

private:
    void getVerificationRecord(const BasicProfile& user, const QString& issuerDid, const QString& collection, const QString& rkey);
    bool sameVerifiers(const QStringList& dids) const;
    void clearCaches();

    Constellation& mConstellation;
    std::vector<QString> mVerifierDids;
    std::unordered_map<QString, QString> mVerifierDidIndex; // did -> list item uri
    ExpiryCache<QString, bool> mIsVerifiedCache;
    QCache<QString, VerificationView::List> mVerificationCache;
    QString mListUri;
};

}
