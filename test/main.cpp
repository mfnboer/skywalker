// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test_post_feed_model.h"
#include "test_search_utils.h"
#include <QObject>
#include <QTest>

int main(int argc, char *argv[])
{
    int status = 0;
    auto runTest = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
    };

    runTest(std::make_unique<TestSearchUtils>().get());
    runTest(std::make_unique<TestPostFeedModel>().get());

    return status;
}
