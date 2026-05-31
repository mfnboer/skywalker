// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "external_source_theme.h"

namespace Skywalker {

ExternalSourceTheme::ExternalSourceTheme(const ATProto::AppBskyEmbed::ViewExternalSourceTheme::SharedPtr& theme) :
    mTheme(theme)
{
}

QColor ExternalSourceTheme::getBackgroundRGB() const
{
    return (mTheme && mTheme->mBackgroundRGB) ? mTheme->mBackgroundRGB->toColor() : QColor{};
}

QColor ExternalSourceTheme::getForegroundRGB() const
{
    return (mTheme && mTheme->mForegroundRGB) ? mTheme->mForegroundRGB->toColor() : QColor{};
}

QColor ExternalSourceTheme::getAccentRGB() const
{
    return (mTheme && mTheme->mAccentRGB) ? mTheme->mAccentRGB->toColor() : QColor{};
}

QColor ExternalSourceTheme::getAccentForegroundRGB() const
{
    return (mTheme && mTheme->mAccentForegroundRGB) ? mTheme->mAccentForegroundRGB->toColor() : QColor{};
}

}
