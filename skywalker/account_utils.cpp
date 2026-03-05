// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "account_utils.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

AccountUtils::AccountUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
}

void AccountUtils::confirmEmail(const QString& token)
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

    qDebug() << "Confirm email:" << *session->mEmail;
    setUpdateInProgress(true);

    bskyClient()->confirmEmail(*session->mEmail, token,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            bskyClient()->updateSessionEmailConfirmed(true);
            emit confirmEmailOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Email confirmation failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit confirmEmailFailed(msg);
        });
}

void AccountUtils::requestEmailConfirmation()
{
    Q_ASSERT(bskyClient());
    qDebug() << "Request email confirmation";

    if (mUpdateInProgress)
        return;

    setUpdateInProgress(true);

    bskyClient()->requestEmailConfirmation(
        [this, presence=getPresence()]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            emit requestEmailConfirmationOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Request email confirmation failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit requestEmailConfirmationFailed(msg);
        });
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

    qDebug() << "Update 2FA:" << enable << "email:" << *session->mEmail;
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

void AccountUtils::updateEmail(const QString& email, const QString& token)
{
    Q_ASSERT(bskyClient());

    if (mUpdateInProgress)
        return;

    const ATProto::ComATProtoServer::Session* session = bskyClient()->getSession();

    if (!session)
        return;

    qDebug() << "Update email:" << email;
    setUpdateInProgress(true);

    bskyClient()->updateEmail(email, {}, Utils::makeOptionalString(token),
        [this, presence=getPresence(), email]{
            if (!presence)
                return;

            setUpdateInProgress(false);
            bskyClient()->updateSessionEmail(email);
            emit updateEmailOk(email);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update email failed:" << error << "-" << msg;
            setUpdateInProgress(false);
            emit updateEmailFailed(msg);
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

void AccountUtils::resetPassword(const QString& password, const QString& token, const QString& host)
{
    if (mUpdateInProgress)
        return;

    ATProto::Client* bsky = bskyClient();

    if (!bsky)
    {
        if (host.isEmpty())
        {
            qWarning() << "Host is missing";
            QTimer::singleShot(0, this, [this]{ emit resetPasswordFailed(tr("Hosting provider is missing")); });
            return;
        }

        bsky = bskyClientNotLoggedIn(host);
    }

    Q_ASSERT(bsky);

    qDebug() << "Reset password";
    setUpdateInProgress(true);

    bsky->resetPassword(password, token,
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

void AccountUtils::requestPasswordReset(QString email, const QString& host)
{
    if (mUpdateInProgress)
        return;

    ATProto::Client* bsky = bskyClient();

    if (!bsky)
    {
        if (host.isEmpty())
        {
            qWarning() << "Host is missing";
            QTimer::singleShot(0, this, [this]{ emit requestResetPasswordFailed(tr("Hosting provider is missing")); });
            return;
        }

        bsky = bskyClientNotLoggedIn(host);
    }

    Q_ASSERT(bsky);

    if (email.isEmpty())
    {
        const ATProto::ComATProtoServer::Session* session = bsky->getSession();

        if (!session || !session->mEmail)
        {
            qWarning() << "No email available";
            QTimer::singleShot(0, this, [this]{ emit requestResetPasswordFailed(tr("Email address is missing")); });
            return;
        }

        email = *session->mEmail;
    }

    qDebug() << "Request reset password:" << email;
    setUpdateInProgress(true);

    bsky->requestPasswordReset(email,
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

ATProto::Client* AccountUtils::bskyClientNotLoggedIn(const QString& host)
{
    if (!mBskyClientNotLoggedIn || mHost != host)
    {
        qDebug() << "Create client:" << host;
        auto xrpc = std::make_unique<Xrpc::Client>(host);
        xrpc->setUserAgent(Skywalker::getUserAgentString());
        mBskyClientNotLoggedIn = std::make_shared<ATProto::Client>(std::move(xrpc), this);
        mHost = host;
    }

    return mBskyClientNotLoggedIn.get();
}

}
