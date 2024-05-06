// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "utils.h"

namespace Skywalker::Utils {

std::optional<QString> makeOptionalString(const QString& str)
{
    std::optional<QString> optionalString;
    if (!str.isEmpty())
        optionalString = str;

    return optionalString;
}

}
