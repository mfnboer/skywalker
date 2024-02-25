// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once


namespace Skywalker {

constexpr char const* HOME_FEED = "home";
constexpr char const* INVALID_HANDLE = "handle.invalid";

// RKEY's may not be longer than 15 chars (the spec says more, but the bsky does not allow it)
constexpr char const* RKEY_MUTED_REPOSTS = "sw-muted-repsts";

constexpr char const* COLLECTION_DRAFT_POST = "eu.thereforeiam.skywalker.draft.post";
constexpr char const* DRAFT_DEFS_QUOTE_POST = "eu.thereforeiam.skywalker.draft.defs#quotePost";

}
