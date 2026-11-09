// License: GPLv3
#pragma once
#include "notification.h"
#include <optional>
#include <QString>

namespace Skywalker {

class PostCache;

std::optional<QString> getNotificationBody(const Notification& notification, const PostCache& reasonPostCache);

}
