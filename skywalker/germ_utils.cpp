// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "germ_utils.h"
#include "skywalker.h"

namespace Skywalker {

GermUtils::GermUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

bool GermUtils::mustShowButton(const BasicProfile& profile) const
{
    qDebug() << "Must show button:" << profile.getDid();

    // Do not DM yourself
    if (profile.getDid() == mSkywalker->getUserDid())
        return false;

    const auto germ = profile.getAssociated().getGerm();

    if (germ.isNull())
    {
        qDebug() << "No germ details:" << profile.getDid();
        return false;
    }

    switch (germ.getShowButtonTo())
    {
    case QEnums::SHOW_BUTTON_TO_UNKNOWN:
        qDebug() << "Cannot DM:" << germ.getShowButtonTo();
        return false;
    case QEnums::SHOW_BUTTON_TO_EVERYONE:
        break;
    case QEnums::SHOW_BUTTON_TO_USERS_I_FOLLOW:
        if (!profile.getViewer().isFollowedBy())
        {
            qDebug() << "Not followed by:" << profile.getDid();
            return false;
        }
        break;
    }

    return true;
}

QString GermUtils::getMessageUrl(const BasicProfile& profile) const
{
    const auto germ = profile.getAssociated().getGerm();
    const QString url = createDmUrl(profile, germ.getMessageMeUrl());

    return url;
}

QString GermUtils::createDmUrl(const BasicProfile& profile, const QString& messageMeUrl) const
{
    if (messageMeUrl.isEmpty())
        return {};

    QUrl url{messageMeUrl};

    if (!url.isValid())
    {
        qWarning() << "Invalid url:" << messageMeUrl;
        return {};
    }

    if (url.hasFragment())
    {
        qWarning() << "Url should not have fragment:" << messageMeUrl;
        return {};
    }

    QString path = url.path();
#ifdef Q_OS_ANDROID
    path += "/android";
#else
    path += "/web";
#endif
    const QString fragment = QString("%1+%2").arg(profile.getDid(), mSkywalker->getUserDid());
    url.setPath(path);
    url.setFragment(fragment);
    qDebug() << "DM url:" << url;
    return url.toString();
}

}