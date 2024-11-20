// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "focus_hashtags.h"
#include "post.h"

namespace Skywalker {

class IPostFilter
{
public:
    virtual ~IPostFilter() = default;
    virtual QString getName() const = 0;
    virtual bool match(const Post& post) const = 0;
};

class FocusHashtagsPostFilter : public IPostFilter
{
public:
    explicit FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry);
    QString getName() const override;
    bool match(const Post& post) const override;

private:
    FocusHashtags mFocusHashtags;
};

class AuthorPostFilter : public IPostFilter
{
public:
    AuthorPostFilter(const QString& did, const QString& handle);
    QString getName() const override;
    bool match(const Post& post) const override;

private:
    QString mDid;
    QString mHandle;
};

}
