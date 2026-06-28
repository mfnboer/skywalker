// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "join_link_preview.h"

namespace Skywalker {

JoinLinkPreview::JoinLinkPreview(const ATProto::ChatBskyGroup::JoinLinkPreviewView& view) :
    mConvoId(view.mConvoId),
    mCode(view.mCode),
    mName(view.mName),
    mOwner(*view.mOwner),
    mMemberCount(view.mMemberCount),
    mMemberLimit(view.mMemberLimit),
    mRequireApproval(view.mRequireApproval),
    mJoinRule((QEnums::JoinRule)view.mJoinRule)
{
}

JoinLinkPreview::JoinLinkPreview(const ATProto::ChatBskyGroup::DisabledJoinLinkPreviewView& view) :
    mCode(view.mCode),
    mDisabled(true)
{
}

JoinLinkPreview::JoinLinkPreview(const ATProto::ChatBskyGroup::InvalidJoinLinkPreviewView& view) :
    mCode(view.mCode),
    mInvalid(true)
{
}

JoinLinkPreview::JoinLinkPreview(const ATProto::UnknownVariant& view)
{
    qWarning() << "Unknow join link preview:" << view.mType;
}

}
