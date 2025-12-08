// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "utils.h"
#include "android_utils.h"
#include "definitions.h"
#include "jni_callback.h"
#include "skywalker.h"

namespace Skywalker {

bool Utils::sEmojiPickerShown = false;

Utils::Utils(QObject* parent) :
    WrappedSkywalker(parent)
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::emojiPicked, this,
        [this](QString emoji){ handleEmojiPicked(emoji); }
    );
}

std::optional<QString> Utils::makeOptionalString(const QString& str)
{
    std::optional<QString> optionalString;
    if (!str.isEmpty())
        optionalString = str;

    return optionalString;
}

QColor Utils::determineForegroundColor(const QColor& backgroundColor, const QColor& lightColor, const QColor& darkColor)
{
    const float blackness = backgroundColor.toCmyk().blackF();
    qDebug() << "Blackness:" << blackness;
    return blackness < 0.3 ? lightColor : darkColor;
}

bool Utils::similarColors(const QColor& lhs, const QColor& rhs)
{
    const float blackness = lhs.toCmyk().blackF();
    const bool dark = blackness > 0.3;
    const int threshold = dark ? 24 : 12;
    return std::abs(lhs.red() - rhs.red()) < threshold && std::abs(lhs.green() - rhs.green()) < threshold && std::abs(lhs.blue() - rhs.blue()) < threshold;
}

bool Utils::translate(const QString& text)
{
    return AndroidUtils::translate(text);
}

void Utils::showEmojiPicker()
{
    if (!sEmojiPickerShown)
    {
        Q_ASSERT(mSkywalker);
        const auto displayMode = mSkywalker->getUserSettings()->getActiveDisplayMode();
        AndroidUtils::showEmojiPicker(displayMode);
        sEmojiPickerShown = true;
    }
}

void Utils::dismissEmojiPicker()
{
    if (sEmojiPickerShown)
    {
        AndroidUtils::dismissEmojiPicker();
        sEmojiPickerShown = false;
    }
}

void Utils::handleEmojiPicked(const QString& emoji)
{
    sEmojiPickerShown = false;
    emit emojiPicked(emoji);
}

bool Utils::isFollowingListUri(const QString& uri)
{
    return uri == FOLLOWING_LIST_URI;
}

QString Utils::getFollowingUri()
{
    return FOLLOWING_LIST_URI;
}

}
