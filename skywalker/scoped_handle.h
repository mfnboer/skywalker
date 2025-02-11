// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class ScopedHandle : public QObject
{
    Q_OBJECT

public:
    using ReleaseFun = std::function<void()>;

    explicit ScopedHandle(const ReleaseFun& releaseFun, QObject* parent = nullptr);
    ~ScopedHandle();

private:
    ReleaseFun mReleaseFun;
};

}
