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

    if (mUpdateInProgress)
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
    setUpdateInProgress(true);

    bskyClient()->updateEmail(*session->mEmail, enable, Utils::makeOptionalString(token),
        [this, presence=getPresence(), enable]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            bskyClient()->updateSession2FA(enable);
            emit update2FAOk(enable);
        },
        [this, presence=getPresence(), enable](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update 2FA" << enable << "failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit update2FAFailed(msg);
        });
}

void AccountUtils::requestEmailUpdateToken()
{
    Q_ASSERT(bskyClient());
    qDebug() << "Request email update token";

    if (mUpdateInProgress)
        return;

    setUpdateInProgress(true);

    bskyClient()->requestEmailUpdate(
        [this, presence=getPresence()](ATProto::ComATProtoServer::RequestEmailUpdateOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Requenst email update token, tokenRequired:" << output->mTokenRequired;
            setUpdateInProgress(false);

            if (output->mTokenRequired)
                emit emailUpdateTokenOk();
            else
                emit emailUpdateTokenNotRequired();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Request email update token failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit emailUpdateTokenFailed(msg);
        });
}

void AccountUtils::resetPassword(const QString& password, const QString& token)
{
    Q_ASSERT(bskyClient());

    if (mUpdateInProgress)
        return;

    qDebug() << "Reset password";
    setUpdateInProgress(true);

    bskyClient()->resetPassword(password, token,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            emit resetPasswordOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Reset password failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit resetPasswordFailed(msg);
        });
}

void AccountUtils::requestPasswordReset(QString email)
{
    Q_ASSERT(bskyClient());

    if (mUpdateInProgress)
        return;

    if (email.isEmpty())
    {
        const ATProto::ComATProtoServer::Session* session = bskyClient()->getSession();

        if (!session)
            return;

        if (!session->mEmail)
        {
            qWarning() << "No email available";
            QTimer::singleShot(0, this, [this]{ emit requestResetPasswordFailed(tr("No email address available")); });
            return;
        }

        email = *session->mEmail;
    }

    qDebug() << "Request reset password:" << email;
    setUpdateInProgress(true);

    bskyClient()->requestPasswordReset(email,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            emit requestResetPasswordOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Request reset password failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit requestResetPasswordFailed(msg);
        });
}

void AccountUtils::setUpdateInProgress(bool inProgress)
{
    if (mUpdateInProgress == inProgress)
        return;

    mUpdateInProgress = inProgress;
    emit updateInProgressChanged();
}

}
