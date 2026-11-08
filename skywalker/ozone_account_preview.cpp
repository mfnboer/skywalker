// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_account_preview.h"

namespace Skywalker {

OzoneAccountPreview::OzoneAccountPreview(const ATProto::ToolsOzoneInbox::AccountPreview::SharedPtr& account) :
    mAccount(account)
{
}

RecordContent OzoneAccountPreview::getProfile() const
{
    if (!mAccount || !mAccount->mProfile)
        return {};

    if (ATProto::holdsNonNull<ATProto::AppBskyActor::Profile::SharedPtr>(*mAccount->mProfile))
    {
        auto profile = std::get<ATProto::AppBskyActor::Profile::SharedPtr>(*mAccount->mProfile);
        QString text = profile->mDisplayName.value_or("");

        if (profile->mDescription)
        {
            if (!text.isEmpty())
                text += "\n";

            text += *profile->mDescription;
        }

        return RecordContent{ text, profile->mCreatedAt.value_or(QDateTime{}) };
    }

    return {};
}

}