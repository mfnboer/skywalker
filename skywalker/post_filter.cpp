// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "post_filter.h"
#include "author_cache.h"

namespace Skywalker {

IPostFilter::Ptr IPostFilter::fromJson(const QJsonObject& json, const UpdatedCb& updatedCb)
{
    const ATProto::XJsonObject xjson(json);
    const QString type = xjson.getRequiredString("$type");

    if (type == HashtagPostFilter::TYPE)
        return HashtagPostFilter::fromJson(json);

    if (type == FocusHashtagsPostFilter::TYPE)
        return FocusHashtagsPostFilter::fromJson(json);

    if (type == AuthorPostFilter::TYPE)
        return AuthorPostFilter::fromJson(json, updatedCb);

    if (type == VideoPostFilter::TYPE)
        return VideoPostFilter::fromJson(json);

    if (type == MediaPostFilter::TYPE)
        return MediaPostFilter::fromJson(json);

    qWarning() << "Uknown type:" << type;
    return nullptr;
}

HashtagPostFilter::HashtagPostFilter(const QString& hashtag)
{
    if (hashtag.startsWith('#'))
        mFocusHashtags.addEntry(hashtag.sliced(1));
    else
        mFocusHashtags.addEntry(hashtag);
}

const QString& HashtagPostFilter::getHashtag() const
{
    return mFocusHashtags.getEntries().first()->getHashtags().first();
}

QString HashtagPostFilter::getName() const
{
    return "#" + getHashtag();
}

bool HashtagPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return mFocusHashtags.match(post).first;
}

QJsonObject HashtagPostFilter::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("hashtag", getHashtag());

    return json;
}

HashtagPostFilter::Ptr HashtagPostFilter::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    const QString hashtag = xjson.getRequiredString("hashtag");
    auto filter = std::make_unique<HashtagPostFilter>(hashtag);
    return filter;
}

FocusHashtagsPostFilter::FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry)
{
    const auto json = focusHashtaghEntry.toJson();
    auto* newEntry = FocusHashtagEntry::fromJson(json);
    mFocusHashtags.addEntry(newEntry);
}

const FocusHashtagEntry* FocusHashtagsPostFilter::getFocusHashtagEntry() const
{
    const auto& entries = mFocusHashtags.getEntries();

    if (entries.empty())
    {
        qWarning() << "No entries";
        return nullptr;
    }

    return entries[0];
}

QString FocusHashtagsPostFilter::getName() const
{
    const auto* entry = getFocusHashtagEntry();

    if (!entry)
        return "unknown";

    const auto& hashtags = entry->getHashtags();

    if (hashtags.empty())
    {
        qWarning() << "No hashtags";
        return "no hashtag";
    }

    QString name;

    for (const auto& tag : hashtags)
    {
        if (!name.isEmpty())
            name += ' ';

        name += "#" + tag;
    }

    return name;
}

QJsonObject FocusHashtagsPostFilter::toJson() const
{
    const FocusHashtagEntry* focusHashtagEntry = getFocusHashtagEntry();
    Q_ASSERT(focusHashtagEntry);

    QJsonObject json;
    json.insert("$type", TYPE);

    if (focusHashtagEntry)
        json.insert("focusHashtag", focusHashtagEntry->toJson());

    return json;
}

FocusHashtagsPostFilter::Ptr FocusHashtagsPostFilter::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    const auto focusHashtagJson = xjson.getRequiredJsonObject("focusHashtag");
    const FocusHashtagEntry* entry = FocusHashtagEntry::fromJson(focusHashtagJson);
    auto filter = std::make_unique<FocusHashtagsPostFilter>(*entry);
    delete entry;
    return filter;
}

QColor FocusHashtagsPostFilter::getBackgroundColor() const
{
    const auto* entry = getFocusHashtagEntry();

    if (!entry)
        return IPostFilter::getBackgroundColor();

    return entry->getHightlightColor();
}

bool FocusHashtagsPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return mFocusHashtags.match(post).first;
}

AuthorPostFilter::AuthorPostFilter(const BasicProfile& profile) :
    mProfile(profile)
{
}

QString AuthorPostFilter::getName() const
{
    return "@" + mProfile.getHandle();
}

BasicProfile AuthorPostFilter::getAuthor() const
{
    return mProfile;
}

bool AuthorPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return post.getAuthor().getDid() == mProfile.getDid();
}

QJsonObject AuthorPostFilter::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("did", mProfile.getDid());
    json.insert("handle", mProfile.getHandle());

    return json;
}

AuthorPostFilter::Ptr AuthorPostFilter::fromJson(const QJsonObject& json, const UpdatedCb& updatedCb)
{
    const ATProto::XJsonObject xjson(json);
    const QString did = xjson.getRequiredString("did");
    const auto* profile = AuthorCache::instance().get(did);

    if (profile)
        return std::make_unique<AuthorPostFilter>(BasicProfile(*profile));

    // Make temporary profile till we have a complete profile
    const QString handle = xjson.getRequiredString("handle");
    auto filter = std::make_unique<AuthorPostFilter>(BasicProfile(did, handle, "", ""));

    AuthorCache::instance().putProfile(did,
        [postFilter=filter.get(), presence=filter->getPresence(), did, updatedCb]{
            if (!presence)
                return;

            const auto* p = AuthorCache::instance().get(did);

            if (p)
            {
                postFilter->mProfile = BasicProfile(*p);

                if (updatedCb)
                    updatedCb(postFilter);
            }
            else
            {
                qWarning() << "Profile missing:" << did;
            }
        });

    return filter;
}

QString VideoPostFilter::getName() const
{
    return QObject::tr("Video");
}

bool VideoPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return post.getVideoView() != nullptr;
}

QJsonObject VideoPostFilter::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);

    return json;
}

VideoPostFilter::Ptr VideoPostFilter::fromJson(const QJsonObject&)
{
    return std::make_unique<VideoPostFilter>();
}

QString MediaPostFilter::getName() const
{
    return QObject::tr("Media");
}

bool MediaPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;


    return !post.getImages().empty() || post.getVideoView() != nullptr;
}

QJsonObject MediaPostFilter::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);

    return json;
}

MediaPostFilter::Ptr MediaPostFilter::fromJson(const QJsonObject&)
{
    return std::make_unique<MediaPostFilter>();
}

}
