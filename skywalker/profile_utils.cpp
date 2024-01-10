// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "profile_utils.h"

namespace Skywalker {

ProfileUtils::ProfileUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
}

void ProfileUtils::getProfileView(const QString& atId)
{
    if (!bskyClient())
        return;

    bskyClient()->getProfile(atId,
        [this, presence=getPresence()](auto profile){
            if (!presence)
                return;

            auto shared = ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr(profile.release());
            emit profileViewOk(Profile(shared));
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getProfileView failed:" << error << " - " << msg;
            emit profileViewFailed(msg);
        });
}

}
