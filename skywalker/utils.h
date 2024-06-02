// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QColor>
#include <QString>

namespace Skywalker::Utils {

std::optional<QString> makeOptionalString(const QString& str);
QColor determineForegroundColor(const QColor& background, const QColor& lightColor, const QColor& darkColor);

}
