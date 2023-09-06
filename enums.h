// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once

#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

// enums that can be used in QML
class QEnums : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum PostType
    {
        POST_STANDALONE = 0,
        POST_ROOT,
        POST_REPLY,
        POST_LAST_REPLY,
        POST_THREAD_ENTRY,
        POST_THREAD_TOP,
        POST_THREAD_PARENT,
        POST_THREAD_FIRST_CHILD,
        POST_THREAD_CHILD,
        POST_THREAD_GRANT_CHILD,
        POST_THREAD_LEAF
    };
    Q_ENUM(PostType)

    enum StatusLevel
    {
        STATUS_LEVEL_INFO = 0,
        STATUS_LEVEL_ERROR
    };
    Q_ENUM(StatusLevel)
};

}
