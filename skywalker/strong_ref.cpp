// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "strong_ref.h"

namespace Skywalker {

StrongRef::StrongRef(const ATProto::ComATProtoRepo::StrongRef::SharedPtr& ref) :
    mRef(ref)
{
}

StrongRef::List StrongRef::makeList(const ATProto::ComATProtoRepo::StrongRef::List& refs)
{
    List refList;
    refList.reserve(refs.size());

    for (const auto& ref : refs)
        refList.push_back(StrongRef(ref));

    return refList;
}

ATProto::ComATProtoRepo::StrongRef::List StrongRef::toATProtoList(const List& strongRefs)
{
    ATProto::ComATProtoRepo::StrongRef::List list;
    list.reserve(strongRefs.size());

    for (const auto& ref : strongRefs)
        list.push_back(ref.getRef());

    return list;
}

QString StrongRef::getUri() const
{
    return mRef ? mRef->mUri : "";
}

QString StrongRef::getCid() const
{
    return mRef ? mRef->mCid : "";
}

ATProto::ComATProtoRepo::StrongRef::SharedPtr StrongRef::getRef() const
{
    return mRef;
}

}
