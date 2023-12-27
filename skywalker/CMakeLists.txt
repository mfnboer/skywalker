set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Quick QuickControls2)

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)

    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(-fprofile-arcs -ftest-coverage)
endif()

include_directories(..)
#qt_add_library(libskywalker)

qt_add_qml_module(libskywalker
    URI skywalker
    VERSION ${PROJECT_VERSION}
    IMPORT_PATH ..
    SOURCES
        skywalker.h
        skywalker.cpp
        post_feed_model.h
        post_feed_model.cpp
        post.h
        profile.h
        post.cpp
        profile.cpp
        image_view.h
        external_view.h
        record_view.h
        record_view.cpp
        record_with_media_view.h
        record_with_media_view.cpp
        external_view.cpp
        enums.h
        svg_outline.h
        svg_image.h
        svg_filled.h
        abstract_post_feed_model.h
        abstract_post_feed_model.cpp
        post_thread_model.h
        post_thread_model.cpp
        profile_store.h
        profile_store.cpp
        photo_picker.h
        photo_picker.cpp
        jni_callback.cpp
        post_utils.h
        post_utils.cpp
        link_card_reader.h
        link_card_reader.cpp
        link_card.h
        image_reader.h
        image_reader.cpp
        presence.h
        local_post_model_changes.h
        local_post_model_changes.cpp
        notification.h
        notification.cpp
        post_record.h
        post_record.cpp
        notification_list_model.h
        notification_list_model.cpp
        post_cache.h
        post_cache.cpp
        author_cache.h
        author_cache.cpp
        author_feed_model.h
        author_feed_model.cpp
        item_store.h
        graph_utils.h
        graph_utils.cpp
        author_list_model.h
        author_list_model.cpp
        local_author_model_changes.h
        local_author_model_changes.cpp
        font_downloader.h
        font_downloader.cpp
    QML_FILES
        Main.qml
        Login.qml
        Avatar.qml
        ImagePreview1.qml
        ImagePreview2.qml
        ImagePreview3.qml
        ImagePreview4.qml
        ExternalView.qml
        RecordView.qml
        PostHeader.qml
        PostBody.qml
        RecordWithMediaView.qml
        RoundedFrame.qml
        ThumbImageView.qml
        FullImageView.qml
        SvgImage.qml
        StatIcon.qml
        StatusPopup.qml
        TimelineView.qml
        SvgButton.qml
        PostFeedViewDelegate.qml
        PostThreadView.qml
        ComposePost.qml
        GuiSettings.qml
        LinkCardView.qml
        QuotePost.qml
        NotificationViewDelegate.qml
        NotificationListView.qml
        PostStats.qml
        SkyFooter.qml
        AuthorView.qml
        AuthorViewDelegate.qml
        AuthorListView.qml
        SkyLabel.qml
        SkyButton.qml
        Message.qml
        PostButton.qml
    RESOURCES
        QML_FILES ContentLabels.qml
        QML_FILES ReplyToRow.qml
        SOURCES content_filter.h
        SOURCES content_filter.cpp
        SOURCES
        QML_FILES FilteredImageWarning.qml
        SOURCES link_utils.h
        SOURCES link_utils.cpp
        SOURCES edit_user_preferences.h
        SOURCES content_group_list_model.h
        SOURCES content_group_list_model.cpp
        SOURCES edit_user_preferences.cpp
        QML_FILES SettingsDrawer.qml
        QML_FILES SkyMenuItem.qml
        QML_FILES SettingsForm.qml
        QML_FILES ContentFilterSettings.qml
        SOURCES enums.cpp
        QML_FILES SkyRadioButton.qml
        SOURCES search_utils.h
        SOURCES search_utils.cpp
        QML_FILES SimpleAuthorListView.qml
        QML_FILES SearchView.qml
        SOURCES search_post_feed_model.h
        SOURCES search_post_feed_model.cpp
        QML_FILES ImageAutoRetry.qml
        SOURCES facet_highlighter.h
        SOURCES facet_highlighter.cpp
        SOURCES wrapped_skywalker.h
        SOURCES wrapped_skywalker.cpp
        SOURCES password_encryption.h
        SOURCES password_encryption.cpp
        SOURCES user_settings.h
        SOURCES user_settings.cpp
        QML_FILES SkyTextInput.qml
        QML_FILES SignIn.qml
        QML_FILES SelectSignInUser.qml
        QML_FILES SwitchUserDrawer.qml
        QML_FILES AltTextEditor.qml
        QML_FILES MenuItemSvg.qml
        SOURCES invite_code.h
        SOURCES invite_code.cpp
        QML_FILES InviteCodesView.qml
        SOURCES invite_code_store.h
        SOURCES invite_code_store.cpp
        SOURCES bookmarks.h
        SOURCES bookmarks.cpp
        SOURCES bookmarks_model.h
        SOURCES bookmarks_model.cpp
        QML_FILES Bookmarks.qml
        QML_FILES About.qml
        QML_FILES StartupStatus.qml
        QML_FILES SimpleHeader.qml
        SOURCES shared_image_provider.h
        SOURCES shared_image_provider.cpp
        QML_FILES FlickableRefresher.qml
        SOURCES definitions.h
        QML_FILES Report.qml
        SOURCES report_reason.h
        SOURCES report_utils.h
        SOURCES report_utils.cpp
        QML_FILES SkyTextEdit.qml
        RESOURCES
        QML_FILES SimpleButtonHeader.qml
        QML_FILES ReportDetailsEditor.qml
        SOURCES muted_words.h
        SOURCES muted_words.cpp
        QML_FILES MutedWords.qml
        QML_FILES AddMutedWord.qml
        SOURCES normalized_word_index.h
        SOURCES normalized_word_index.cpp
        SOURCES tenor.h
        SOURCES tenor_gif.h
        SOURCES tenor_category.h
        SOURCES tenor.cpp
        QML_FILES SearchHeader.qml
        QML_FILES TenorSearch.qml
        SOURCES tenor_preview_row.h
        SOURCES tenor_preview_row.cpp
        SOURCES tenor_gif_overview_model.h
        SOURCES tenor_gif_overview_model.cpp
        QML_FILES AnimatedImageAutoRetry.qml
        QML_FILES ThumbAnimatedImageView.qml
        QML_FILES AnimatedImagePreview.qml
        QML_FILES FullAnimatedImageView.qml
        QML_FILES AddReplyRestrictions.qml
        SOURCES feed_list_model.h
        SOURCES generator_view.h
        SOURCES generator_view.cpp
        SOURCES feed_list_model.cpp
        QML_FILES SearchFeeds.qml
        QML_FILES GeneratorViewDelegate.qml
        QML_FILES PostFeedView.qml
        QML_FILES FeedAvatar.qml
        QML_FILES ExpandFeedsButton.qml
        SOURCES favorite_feeds.h
        SOURCES favorite_feeds.cpp
        SOURCES local_feed_model_changes.h
        SOURCES local_feed_model_changes.cpp
        SOURCES feed_utils.h
        SOURCES feed_utils.cpp
        QML_FILES FeedDescriptionView.qml
        QML_FILES QuoteFeed.qml
)

if (NOT ANDROID)
    set(COVERAGE_LIB -lgcov)
endif()

target_link_libraries(libskywalker
    PRIVATE libatproto
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    ${COVERAGE_LIB}
)

target_include_directories(libskywalker INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ..)
