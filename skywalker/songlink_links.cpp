// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "songlink_links.h"

namespace Skywalker {

SonglinkInfo::SonglinkInfo(const QString& name, const QString& link, const QString& logo) :
    mName(name),
    mLink(link),
    mLogo(logo)
{
}

SonglinkLinks::SharedPtr SonglinkLinks::fromJson(const QJsonObject& json)
{
    auto links = std::make_shared<SonglinkLinks>();
    ATProto::XJsonObject xjson(json);
    links->mAmazonMusic = getLink(xjson, "amazonMusic");
    links->mAudioMack = getLink(xjson, "audiomack");
    links->mAnghami = getLink(xjson, "mAnghami");
    links->mDeezer = getLink(xjson, "deezer");
    links->mAppleMusic = getLink(xjson, "appleMusic");
    links->mSoundCloud = getLink(xjson, "soundcloud");
    links->mTidal = getLink(xjson, "tidal");
    links->mYouTubeMusic = getLink(xjson, "youtubeMusic");
    links->mSpotify = getLink(xjson, "spotify");
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
    return mAmazonMusic.isEmpty() &&
        mAudioMack.isEmpty() &&
        mAnghami.isEmpty() &&
        mDeezer.isEmpty() &&
        mAppleMusic.isEmpty() &&
        mSoundCloud.isEmpty() &&
        mTidal.isEmpty() &&
        mYouTubeMusic.isEmpty() &&
        mSpotify.isEmpty();
}

SonglinkInfo::List SonglinkLinks::getLinkInfoList() const
{
    SonglinkInfo::List linkInfoList;

    if (!mSpotify.isEmpty())
        linkInfoList.emplace_back("Spotify", mSpotify, "/images/spotify-logo.png");

    if (!mTidal.isEmpty())
        linkInfoList.emplace_back("TIDAL", mTidal, "/images/tidal-logo.png");

    if (!mYouTubeMusic.isEmpty())
        linkInfoList.emplace_back("YouTube Music", mYouTubeMusic, "/images/youtubemusic-logo.png");

    if (!mDeezer.isEmpty())
        linkInfoList.emplace_back("Deezer", mDeezer, "/images/deezer-logo.png");

    if (!mSoundCloud.isEmpty())
        linkInfoList.emplace_back("SoundCloud", mSoundCloud, "/images/soundcloud-logo.png");

    if (!mAppleMusic.isEmpty())
        linkInfoList.emplace_back("Apple Music", mAppleMusic, "/images/applemusic-logo.png");

    if (!mAudioMack.isEmpty())
        linkInfoList.emplace_back("Audiomack", mAudioMack, "/images/audiomack-logo.png");

    if (!mAnghami.isEmpty())
        linkInfoList.emplace_back("Anghami", mAnghami, "/images/anghami-logo.png");

    if (!mAmazonMusic.isEmpty())
        linkInfoList.emplace_back("Amazon Music", mAmazonMusic, "/images/amazonmusic-logo.png");

    return linkInfoList;
}

}
