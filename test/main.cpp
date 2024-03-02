// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "test_hashtag_index.h"
#include "test_muted_words.h"
#include "test_post_feed_model.h"
#include "test_search_utils.h"
#include <QtTest/QTest>

int main(int argc, char *argv[])
{
    QTEST_SET_MAIN_SOURCE_PATH

    TestHashTagIndex testHastTagIndex;
    QTest::qExec(&testHastTagIndex, argc, argv);

    TestMutedWords testMutedWords;
    QTest::qExec(&testMutedWords, argc, argv);

    TestPostFeedModel testPostFeedModel;
    QTest::qExec(&testPostFeedModel, argc, argv);

    TestSearchUtils testSearchUtils;
    QTest::qExec(&testSearchUtils, argc, argv);

    return 0;
}
