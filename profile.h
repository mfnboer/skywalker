// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_actor.h>

namespace Skywalker {

class BasicProfile
{
public:
    explicit BasicProfile(const ATProto::AppBskyActor::ProfileViewBasic* profile);

    QString getName() const;
    QString getAvatarUrl() const;

private:
    const ATProto::AppBskyActor::ProfileViewBasic* mProfile;
};

}
