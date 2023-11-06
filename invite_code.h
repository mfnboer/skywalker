// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <atproto/lib/lexicon/com_atproto_server.h>
#include <QtQmlIntegration>

namespace Skywalker {

class InviteCodeStore;

class InviteCode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code READ getCode CONSTANT FINAL)
    Q_PROPERTY(bool available READ isAvailable CONSTANT FINAL)
    Q_PROPERTY(bool used READ isUsed CONSTANT FINAL)
    Q_PROPERTY(bool disabled READ isDisabled CONSTANT FINAL)
    Q_PROPERTY(QString usedByDid READ getUsedByDid CONSTANT FINAL)
    Q_PROPERTY(QDateTime usedAt READ getUsedAt CONSTANT FINAL)
    Q_PROPERTY(BasicProfile usedBy READ getUsedBy NOTIFY usedByChanged FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt CONSTANT FINAL)
    QML_ELEMENT

public:
    InviteCode() = default;
    InviteCode(const ATProto::ComATProtoServer::InviteCode& code, InviteCodeStore* store);

    Q_INVOKABLE void copyToClipboard();

    const QString& getCode() const { return mCode; }
    bool isAvailable() const { return mAvailable; }
    bool isDisabled() const { return mDisabled; }
    bool isUsed() const { return !mUsedByDid.isEmpty(); }
    const QString& getUsedByDid() const { return mUsedByDid; }
    QDateTime getUsedAt() const { return mUsedAt; }

    // The profile may not be available yet. In that case you will get an empty
    // profile. The usedByChanged signal will be emited once the profile is available.
    const BasicProfile& getUsedBy();

    void setUsedBy(const BasicProfile& profile);
    QDateTime getCreatedAt() const { return mCreatedAt; }
    bool getRetrievingUsedByProfile() const { return mRetrievingUsedByProfile; }
    void setRetrievingUsedByProfile(bool retrieving) { mRetrievingUsedByProfile = retrieving; }

signals:
    void usedByChanged();

private:
    InviteCodeStore* mStore = nullptr;
    QString mCode;
    bool mAvailable = false;
    bool mDisabled = false;
    QString mUsedByDid;
    QDateTime mUsedAt;
    BasicProfile mUsedBy;
    QDateTime mCreatedAt;
    bool mRetrievingUsedByProfile = false;
};

using InviteCodeList = QList<InviteCode*>;

}
