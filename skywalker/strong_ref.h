// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/com_atproto_repo.h>
#include <QtQmlIntegration>

namespace Skywalker {

class StrongRef
{
    Q_GADGET
    QML_VALUE_TYPE(strongref)

public:
    using List = QList<StrongRef>;

    static List makeList(const ATProto::ComATProtoRepo::StrongRef::List& refs);
    static ATProto::ComATProtoRepo::StrongRef::List toATProtoList(const List& strongRefs);

    StrongRef() = default;
    explicit StrongRef(const ATProto::ComATProtoRepo::StrongRef::SharedPtr& ref);

    Q_INVOKABLE bool isNull() const { return mRef == nullptr; }
    QString getUri() const;
    QString getCid() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getRef() const;

private:
    ATProto::ComATProtoRepo::StrongRef::SharedPtr mRef;
};

}
