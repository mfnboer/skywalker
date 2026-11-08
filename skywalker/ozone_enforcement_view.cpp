// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_enforcement_view.h"

namespace Skywalker {

OzoneEnforcementView::OzoneEnforcementView(const ATProto::ToolsOzoneInbox::EnforcementView::SharedPtr& enforcement) :
    mEnforcement(enforcement)
{
}

}