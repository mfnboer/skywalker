// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "test_anniversary.h"
#include "test_filtered_post_feed_model.h"
#include "test_focus_hashtags.h"
#include "test_hashtag_index.h"
#include "test_muted_words.h"
#include "test_post_feed_model.h"
#include "test_search_utils.h"
#include "test_text_differ.h"
#include "test_text_splitter.h"
#include "test_unicode_fonts.h"
#include "test_uri_with_expiry.h"
#include <QtTest/QTest>

int main(int argc, char *argv[])
{
    TestAnniversary testAnniversary;
    QTest::qExec(&testAnniversary, argc, argv);

    TestFocusHashTags testFocusHashtags;
    QTest::qExec(&testFocusHashtags, argc, argv);

    TestHashTagIndex testHastTagIndex;
    QTest::qExec(&testHastTagIndex, argc, argv);

    TestMutedWords testMutedWords;
    QTest::qExec(&testMutedWords, argc, argv);

    TestPostFeedModel testPostFeedModel;
    QTest::qExec(&testPostFeedModel, argc, argv);

    TestFilteredPostFeedModel testFilteredPostFeedModel;
    QTest::qExec(&testFilteredPostFeedModel, argc, argv);

    TestSearchUtils testSearchUtils;
    QTest::qExec(&testSearchUtils, argc, argv);

    TestTextDiffer testTextDiffer;
    QTest::qExec(&testTextDiffer, argc, argv);

    TestTextSplitter testTextSplitter;
    QTest::qExec(&testTextSplitter, argc, argv);

    TestUnicodeFonts testUnicodeFonts;
    QTest::qExec(&testUnicodeFonts, argc, argv);

    TestUriWithExpiry testUriWithExpiry;
    QTest::qExec(&testUriWithExpiry, argc, argv);

    return 0;
}
