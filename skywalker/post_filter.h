// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "focus_hashtags.h"
#include "post.h"

namespace Skywalker {

class IPostFilter
{
public:
    using Ptr = std::unique_ptr<IPostFilter>;
    static Ptr fromJson(const QJsonObject& json);

    virtual ~IPostFilter() = default;
    virtual QString getName() const = 0;
    virtual QColor getBackgroundColor() const { return "transparent"; }
    virtual BasicProfile getAuthor() const { return BasicProfile{}; }
    virtual QEnums::ContentMode getContentMode() const { return QEnums::CONTENT_MODE_UNSPECIFIED; }
    virtual bool mustAddThread() const { return true; }
    virtual bool match(const Post& post) const = 0;
    virtual QJsonObject toJson() const = 0;
};

class HashtagPostFilter : public IPostFilter
{
public:
    using Ptr = std::unique_ptr<HashtagPostFilter>;
    static constexpr char const* TYPE = "hashtagPostFilter";
    static Ptr fromJson(const QJsonObject& json);

    explicit HashtagPostFilter(const QString& hashtag);
    QString getName() const override;
    bool match(const Post& post) const override;
    QJsonObject toJson() const override;

private:
    const QString& getHashtag() const;
    FocusHashtags mFocusHashtags;
};

class FocusHashtagsPostFilter : public IPostFilter
{
public:
    using Ptr = std::unique_ptr<FocusHashtagsPostFilter>;
    static constexpr char const* TYPE = "focusHashtagPostFilter";
    static Ptr fromJson(const QJsonObject& json);

    explicit FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry);
    QString getName() const override;
    QColor getBackgroundColor() const override;
    bool match(const Post& post) const override;
    QJsonObject toJson() const override;

private:
    const FocusHashtagEntry* getFocusHashtagEntry() const;

    FocusHashtags mFocusHashtags;
};

class AuthorPostFilter : public IPostFilter
{
public:
    using Ptr = std::unique_ptr<AuthorPostFilter>;
    static constexpr char const* TYPE = "authorPostFilter";
    static Ptr fromJson(const QJsonObject& json);

    AuthorPostFilter(const BasicProfile& profile);
    QString getName() const override;
    BasicProfile getAuthor() const override;
    bool match(const Post& post) const override;
    QJsonObject toJson() const override;

private:
    BasicProfile mProfile;
};

class VideoPostFilter : public IPostFilter
{
public:
    using Ptr = std::unique_ptr<VideoPostFilter>;
    static constexpr char const* TYPE = "videoPostFilter";
    static Ptr fromJson(const QJsonObject& json);

    QString getName() const override;
    QEnums::ContentMode getContentMode() const override { return QEnums::CONTENT_MODE_VIDEO; }
    bool mustAddThread() const override { return false; }
    bool match(const Post& post) const override;
    QJsonObject toJson() const override;
};

class MediaPostFilter : public IPostFilter
{
public:
    using Ptr = std::unique_ptr<MediaPostFilter>;
    static constexpr char const* TYPE = "mediaPostFilter";
    static Ptr fromJson(const QJsonObject& json);

    QString getName() const override;
    QEnums::ContentMode getContentMode() const override { return QEnums::CONTENT_MODE_MEDIA; }
    bool mustAddThread() const override { return false; }
    bool match(const Post& post) const override;
    QJsonObject toJson() const override;
};

}
