// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace Skywalker {

constexpr char const* UI_LANGUAGE = "en";

constexpr char const* SKYWALKER_HANDLE = "@skywalker.thereforeiam.eu";
constexpr char const* HOME_FEED = "home";
constexpr char const* INVALID_HANDLE = "handle.invalid";

// RKEY's may not be longer than 15 chars (the spec says more, but the bsky does not allow it)
constexpr char const* RKEY_MUTED_REPOSTS = "sw-muted-repsts";

}
