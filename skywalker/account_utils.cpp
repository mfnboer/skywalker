// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "account_utils.h"
#include "utils.h"

namespace Skywalker {

AccountUtils::AccountUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
}

void AccountUtils::update2FA(bool enable, const QString& token)
{
    Q_ASSERT(bskyClient());

    if (mEmailUpdateInProgress)
        return;

    const ATProto::ComATProtoServer::Session* session = bskyClient()->getSession();

    if (!session)
        return;

    if (!session->mEmail)
    {
        qWarning() << "No email available";
        QTimer::singleShot(0, this, [this]{ emit update2FAFailed(tr("No email address available")); });
        return;
    }

    qDebug() << "Update 2FA:" << enable << "email:" << enable;
    setEmailUpdateInProgress(true);

    bskyClient()->updateEmail(*session->mEmail, enable, Utils::makeOptionalString(token),
        [this, presence=getPresence(), enable]{
            if (!presence)
                return;

            setEmailUpdateInProgress(false);
            bskyClient()->updateSession2FA(enable);
            emit update2FAOk(enable);
        },
        [this, presence=getPresence(), enable](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update 2FA" << enable << "failed:" << error << "-" << msg;
            setEmailUpdateInProgress(false);
            emit update2FAFailed(msg);
        });
}

void AccountUtils::requestEmailUpdateToken()
{
    Q_ASSERT(bskyClient());
    qDebug() << "Request email update token";

    if (mEmailUpdateInProgress)
        return;

    setEmailUpdateInProgress(true);

    bskyClient()->requestEmailUpdate(
        [this, presence=getPresence()](ATProto::ComATProtoServer::RequestEmailUpdateOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Requenst email update token, tokenRequired:" << output->mTokenRequired;
            setEmailUpdateInProgress(false);

            if (output->mTokenRequired)
                emit emailUpdateTokenOk();
            else
                emit emailUpdateTokenNotRequired();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Request email update token failed:" << error << "-" << msg;
            setEmailUpdateInProgress(false);
            emit emailUpdateTokenFailed(msg);
        });
}

void AccountUtils::setEmailUpdateInProgress(bool inProgress)
{
    if (mEmailUpdateInProgress == inProgress)
        return;

    mEmailUpdateInProgress = inProgress;
    emit emailUpdateInProgressChanged();
}

}
