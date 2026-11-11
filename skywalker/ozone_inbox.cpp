// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_inbox.h"

namespace Skywalker {

OzoneInbox::OzoneInbox(QObject* parent) : WrappedSkywalker(parent)
{
}

void OzoneInbox::reset()
{
    setUserAccountStatus({});
}

void OzoneInbox::updateUserAccountStatus()
{
    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetAccountStatus(
        [this](ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr output){
            const OzoneAccountStatus status(output);
            setUserAccountStatus(status);
        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get user standing:" << error << " - " << msg;
        });
}

void OzoneInbox::setUserAccountStatus(const OzoneAccountStatus& status)
{
    mUserAccountStatus = status;
    emit userAccountStatusChanged();
}

}
