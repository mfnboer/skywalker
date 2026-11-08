// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_policy_view.h"

namespace Skywalker {

OzonePolicyView::OzonePolicyView(const ATProto::ToolsOzoneInbox::PolicyView::SharedPtr& policy) :
    mPolicy(policy)
{
}

OzonePolicyView::List OzonePolicyView::makeList(const ATProto::ToolsOzoneInbox::PolicyView::List& policies)
{
    OzonePolicyView::List policyList;
    policyList.reserve(policies.size());

    for (const auto& policy : policies)
        policyList.push_back(OzonePolicyView(policy));

    return policyList;
}

}