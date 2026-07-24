// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace Skywalker {

constexpr char const* UI_LANGUAGE = "en";

constexpr char const* SKYWALKER_HANDLE = "skywalker.thereforeiam.eu";
constexpr char const* HOME_FEED = "home";
constexpr char const* INVALID_HANDLE = "handle.invalid";
constexpr char const* INVALID_HANDLE_SUFFIX = ".invalid";

constexpr char const* EUROSKY_DOMAN = "eurosky.social";
constexpr char const* EUROSKY_DID = "did:plc:ooensn4mr5mhznzypvxelfa3";
constexpr char const* EUROSKY_HANDLE = "eurosky.social";
constexpr char const* EUROSKY_NAME = "Eurosky Social";

// RKEY's may not be longer than 15 chars (the spec says more, but the bsky does not allow it)
// It turns out that this RKEY is invalid. The RKEY for a list must be a TID. In that past
// the PDS did not validate the RKEY format and allowed any string.
constexpr char const* RKEY_MUTED_REPOSTS_DEPRECATED = "sw-muted-repsts";

constexpr char const* LIST_NAME_MUTED_REPOSTS = "Skywalker muted reposts";
constexpr char const* LIST_NAME_TRUSTED_VERIFIERS = "Skywalker trusted verifiers";

// This is not really an uri, but we use it as a a uri to indicate the list of user you follow
constexpr char const* FOLLOWING_LIST_URI = "following";

}
