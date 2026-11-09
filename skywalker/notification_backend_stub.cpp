// License: GPLv3
#include "notification_backend.h"

namespace Skywalker {
namespace {

class NullNotificationBackend final : public NotificationBackend
{
public:
    using NotificationBackend::NotificationBackend;

    void notify(const QString& title, const QString& body) override
    {
        Q_UNUSED(title);
        Q_UNUSED(body);
    }
};

}

NotificationBackend* createNotificationBackend(QObject* parent)
{
    return new NullNotificationBackend(parent);
}

}
