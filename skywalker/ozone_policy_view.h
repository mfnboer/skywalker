// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzonePolicyView
{
    Q_GADGET
    Q_PROPERTY(QString key READ getKey FINAL)
    Q_PROPERTY(QString displayName READ getDisplayName FINAL)
    Q_PROPERTY(QString link READ getLink FINAL)
    QML_VALUE_TYPE(ozonepolicyview)

public:
    using List = QList<OzonePolicyView>;

    static List makeList(const ATProto::ToolsOzoneInbox::PolicyView::List& policies);

    OzonePolicyView() = default;
    explicit OzonePolicyView(const ATProto::ToolsOzoneInbox::PolicyView::SharedPtr& policy);

    Q_INVOKABLE bool isNull() const { return mPolicy == nullptr; }
    QString getKey() const { return mPolicy ? mPolicy->mKey : ""; }
    QString getDisplayName() const { return mPolicy ? mPolicy->mDisplayName : ""; }
    QString getLink() const { return mPolicy ? mPolicy->mLink : ""; }

private:
    ATProto::ToolsOzoneInbox::PolicyView::SharedPtr mPolicy;
};

}