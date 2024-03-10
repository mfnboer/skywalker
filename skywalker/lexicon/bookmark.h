// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>

namespace Skywalker::Bookmark {

// eu.thereforeiam.skywalker.bookmark
struct Bookmark {
    QString mUri;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Bookmark>;
    static Ptr fromJson(const QJsonObject& json);
};

}
