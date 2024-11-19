// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "post_filter.h"

namespace Skywalker {

FocusHashtagsPostFilter::FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry)
{
    const auto json = focusHashtaghEntry.toJson();
    auto* newEntry = FocusHashtagEntry::fromJson(json);
    mFocusHashtags.addEntry(newEntry);
}

bool FocusHashtagsPostFilter::match(const Post& post) const
{
    return mFocusHashtags.match(post);
}

}
