// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "record_content.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneAccountPreview
{
    Q_GADGET
    Q_PROPERTY(QString handle READ getHandle FINAL)
    Q_PROPERTY(RecordContent profile READ getProfile FINAL)
    QML_VALUE_TYPE(ozoneaccountpreview)

public:
    OzoneAccountPreview() = default;
    explicit OzoneAccountPreview(const ATProto::ToolsOzoneInbox::AccountPreview::SharedPtr& account);

    Q_INVOKABLE bool isNull() const { return mAccount == nullptr; }
    QString getHandle() const { return (mAccount && mAccount->mHandle) ? *mAccount->mHandle : ""; }
    RecordContent getProfile() const;

private:
    ATProto::ToolsOzoneInbox::AccountPreview::SharedPtr mAccount;
};

}