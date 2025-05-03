// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "songlink_links.h"

namespace Skywalker {

const std::unordered_map<QString, SonglinkLinks::SonglinkPlatform> SonglinkLinks::SONGLINK_PLATFORM_MAP = {
    { "spotify", { "Spotify", "/images/spotify-logo.png", { "open.spotify.com" }, { "/album/", "/track/" }, 1 } },
    { "tidal", { "TIDAL", "/images/tidal-logo.png", { "tidal.com", "listen.tidal.com" }, {}, 2 } },
    { "youtubeMusic", { "YouTube Music", "/images/youtubemusic-logo.png", { "music.youtube.com" }, {}, 3 } },
    { "deezer", { "Deezer", "/images/deezer-logo.png", { "www.deezer.com" }, {}, 4 } },
    { "soundcloud", { "SoundCloud", "/images/soundcloud-logo.png", { "soundcloud.com" }, {}, 5 } },
    { "appleMusic", { "Apple Music", "/images/applemusic-logo.png", { "geo.music.apple.com" }, {}, 6 } },
    { "audiomack", { "Audiomack", "/images/audiomack-logo.png", { "audiomack.com" }, {}, 7 } },
    { "anghami", { "Anghami", "/images/anghami-logo.png", { "play.anghami.com" }, {}, 8 } },
    { "boomplay", { "Boomplay", "/images/boomplay-logo.png", { "www.boomplay.com" }, {}, 9 } },
    { "amazonMusic", { "Amazon Music", "/images/amazonmusic-logo.png", { "music.amazon.com" }, {}, 10 } }
};

SonglinkInfo::SonglinkInfo(const QString& jsonKey, const QString& link) :
    mJsonKey(jsonKey),
    mLink(link)
{
}

const QString& SonglinkInfo::getName() const
{
    auto it = SonglinkLinks::SONGLINK_PLATFORM_MAP.find(mJsonKey);

    if (it != SonglinkLinks::SONGLINK_PLATFORM_MAP.end())
        return it->second.mName;

    static const QString NULL_STRING;
    return NULL_STRING;
}

const QString& SonglinkInfo::getLogo() const
{
    auto it = SonglinkLinks::SONGLINK_PLATFORM_MAP.find(mJsonKey);

    if (it != SonglinkLinks::SONGLINK_PLATFORM_MAP.end())
        return it->second.mLogo;

    static const QString NULL_STRING;
    return NULL_STRING;
}

int SonglinkInfo::getPrio() const
{
    auto it = SonglinkLinks::SONGLINK_PLATFORM_MAP.find(mJsonKey);

    if (it != SonglinkLinks::SONGLINK_PLATFORM_MAP.end())
        return it->second.mPrio;

    return -1;
}

SonglinkLinks::SharedPtr SonglinkLinks::fromJson(const QJsonObject& json)
{
    auto links = std::make_shared<SonglinkLinks>();
    const auto keys = json.keys();
    ATProto::XJsonObject xjson(json);

    for (const auto& key : keys)
    {
        if (!SONGLINK_PLATFORM_MAP.contains(key))
            continue;

        const QString link = getLink(xjson, key);
        links->mLinkInfoList.emplace_back(key, link);
    }

    std::sort(links->mLinkInfoList.begin(), links->mLinkInfoList.end(),
        [](const SonglinkInfo& lhs, const SonglinkInfo& rhs){
            return lhs.getPrio() < rhs.getPrio();
        });

    return links;
}

QString SonglinkLinks::getLink(const ATProto::XJsonObject& xjson, const QString& platform)
{
    auto platformJson = xjson.getOptionalJsonObject(platform);

    if (!platformJson)
        return {};

    ATProto::XJsonObject platformXJson(*platformJson);
    return platformXJson.getRequiredString("url");
}

bool SonglinkLinks::isNull() const
{
    return mLinkInfoList.empty();
}

std::unordered_map<QString, QStringList> SonglinkLinks::getPlatformHosts()
{
    std::unordered_map<QString, QStringList> hosts;

    for (const auto& [_, platform] : SONGLINK_PLATFORM_MAP)
    {
        for (const auto& host : platform.mHosts)
            hosts[host] = platform.mPaths;
    }

    return hosts;
}

}
