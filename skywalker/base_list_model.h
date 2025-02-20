// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QList>

namespace Skywalker {

class BaseListModel
{
public:
    virtual void refreshAllData() { changeData({}); }

protected:
    virtual void changeData(const QList<int>& roles) = 0;
};

}
