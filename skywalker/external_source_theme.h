// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker {

class ExternalSourceTheme
{
    Q_GADGET
    Q_PROPERTY(QColor backgroundRGB READ getBackgroundRGB FINAL)
    Q_PROPERTY(QColor foregroundRGB READ getForegroundRGB FINAL)
    Q_PROPERTY(QColor accentRGB READ getAccentRGB FINAL)
    Q_PROPERTY(QColor accentForegroundRGB READ getAccentForegroundRGB FINAL)
    QML_VALUE_TYPE(externalsourcetheme)

public:
    ExternalSourceTheme() = default;
    explicit ExternalSourceTheme(const ATProto::AppBskyEmbed::ViewExternalSourceTheme::SharedPtr& theme);

    Q_INVOKABLE bool isNull() const { return mTheme == nullptr; }
    QColor getBackgroundRGB() const;
    QColor getForegroundRGB() const;
    QColor getAccentRGB() const;
    QColor getAccentForegroundRGB() const;

private:
    ATProto::AppBskyEmbed::ViewExternalSourceTheme::SharedPtr mTheme;
};

}
