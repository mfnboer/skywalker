// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test_search_utils.h"
#include <QObject>
#include <QTest>

int main(int argc, char *argv[])
{
    int status = 0;
    auto runTest = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
    };

    runTest(new TestSearchUtils);

    return status;
}
