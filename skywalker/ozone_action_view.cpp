// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_action_view.h"

namespace Skywalker {

OzoneActionView::OzoneActionView(const ATProto::ToolsOzoneInbox::ActionView::SharedPtr& action) :
    mAction(action)
{
}

OzoneActionView::List OzoneActionView::makeList(const ATProto::ToolsOzoneInbox::ActionView::List& actions)
{
    OzoneActionView::List actionList;
    actionList.reserve(actions.size());

    for (const auto& action : actions)
        actionList.push_back(OzoneActionView(action));

    return actionList;
}

}