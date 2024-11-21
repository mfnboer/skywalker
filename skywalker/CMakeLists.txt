set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Svg)

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

set(QT_QML_GENERATE_QMLLS_INI "ON" CACHE STRING "qmlls.ini")
set(QML_IMPORT_PATH "${CMAKE_BINARY_DIR}" CACHE STRING "QML imports.")

include_directories(..)

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
        jni_callback.h
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
        SkySvg.qml
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
        QML_FILES PostFeedHeader.qml
        QML_FILES ListListView.qml
        QML_FILES SimpleDescriptionHeader.qml
        SOURCES list_list_model.h
        SOURCES list_view.h
        SOURCES list_view.cpp
        SOURCES list_list_model.cpp
        QML_FILES ListViewDelegate.qml
        QML_FILES ListAvatar.qml
        QML_FILES EmptyListIndication.qml
        QML_FILES EditList.qml
        QML_FILES SkyFormattedTextEdit.qml
        QML_FILES AuthorTypeaheadView.qml
        QML_FILES TextLengthBar.qml
        QML_FILES TextLengthCounter.qml
        QML_FILES EditAvatar.qml
        QML_FILES ImageFileDialog.qml
        QML_FILES PostListFeedView.qml
        QML_FILES ListFeedDescriptionView.qml
        QML_FILES QuoteList.qml
        QML_FILES SearchAuthor.qml
        SOURCES profile_utils.h
        SOURCES profile_utils.cpp
        SOURCES favorite_feed_view.h
        SOURCES favorite_feed_view.cpp
        QML_FILES UserListsPage.qml
        QML_FILES ModerationListsPage.qml
        SOURCES local_list_model_changes.h
        SOURCES local_list_model_changes.cpp
        QML_FILES ListViewerState.qml
        QML_FILES FavoriteStar.qml
        SOURCES list_view_include.h
        QML_FILES AddUserListListView.qml
        QML_FILES AddUserListViewDelegate.qml
        SOURCES gif_utils.h
        SOURCES gif_utils.cpp
        QML_FILES AuthorPostsList.qml
        QML_FILES EditProfile.qml
        SOURCES local_profile_changes.h
        SOURCES local_profile_changes.cpp
        SOURCES unicode_fonts.h
        SOURCES unicode_fonts.cpp
        QML_FILES PagingComboBox.qml
        QML_FILES AddContentWarning.qml
        SOURCES content_label.h
        QML_FILES ContentLabelInfo.qml
        QML_FILES AccessibleImage.qml
        QML_FILES AccessibilityUtils.qml
        QML_FILES AccessibleMenuItem.qml
        QML_FILES AccessibleTabButton.qml
        QML_FILES CloseMenuItem.qml
        QML_FILES AccessibleCheckBox.qml
        QML_FILES AccessibleText.qml
        QML_FILES AccessibleSwitch.qml
        SOURCES emoji_fix_highlighter.h
        SOURCES emoji_fix_highlighter.cpp
        QML_FILES SkyCleanedText.qml
        SOURCES grapheme_info.h
        SOURCES grapheme_info.cpp
        SOURCES file_utils.h
        SOURCES file_utils.cpp
        SOURCES draft_posts.h
        SOURCES draft_posts.cpp
        SOURCES draft_posts_model.h
        SOURCES draft_posts_model.cpp
        QML_FILES DraftPostsView.qml
        QML_FILES DraftPostViewDelegate.qml
        SOURCES draft_post_data.h
        SOURCES atproto_image_provider.h
        SOURCES atproto_image_provider.cpp
        SOURCES hashtag_index.h
        SOURCES hashtag_index.cpp
        QML_FILES HashtagTypeaheadView.qml
        QML_FILES HashtagListView.qml
        QML_FILES SearchPostScope.qml
        SOURCES lexicon/bookmark.h
        SOURCES lexicon/bookmark.cpp
        SOURCES lexicon/lexicon.h
        SOURCES lexicon/draft.h
        SOURCES lexicon/draft.cpp
        SOURCES android_utils.h
        SOURCES android_utils.cpp
        SOURCES jni_utils.h
        SOURCES jni_utils.cpp
        SOURCES offline_message_checker.h
        SOURCES offline_message_checker.cpp
        SOURCES draft_posts_migration.h
        SOURCES draft_posts_migration.cpp
        QML_FILES FontComboBox.qml
        QML_FILES AddGifButton.qml
        QML_FILES ImageScroller.qml
        QML_FILES SvgTransparentButton.qml
        QML_FILES ComposePostItem.qml
        QML_FILES SeparatorLine.qml
        QML_FILES VirtualKeyboardPageResizer.qml
        SOURCES tenor_gif.cpp
        QML_FILES LanguageComboBox.qml
        SOURCES language_utils.h
        SOURCES language_utils.cpp
        QML_FILES LanguageComboCheckBox.qml
        QML_FILES LanguageLabels.qml
        QML_FILES ModeratorIcon.qml
        SOURCES labeler.h
        SOURCES labeler.cpp
        QML_FILES ContentGroupDelegate.qml
        SOURCES content_group.h
        SOURCES content_group.cpp
        QML_FILES StatAuthors.qml
        QML_FILES HeaderText.qml
        QML_FILES ReportAppeal.qml
        SOURCES utils.h
        SOURCES utils.cpp
        QML_FILES AuthorComboBox.qml
        QML_FILES AuthorItemDelegate.qml
        SOURCES content_label.cpp
        SOURCES anniversary_card.h
        SOURCES anniversary_card.cpp
        QML_FILES AnniversaryCardMaker.qml
        QML_FILES ColorSelector.qml
        SOURCES anniversary.h
        SOURCES anniversary.cpp
        QML_FILES BadgeCounter.qml
        SOURCES chat_profile.h
        SOURCES chat_profile.cpp
        SOURCES convo_view.h
        SOURCES convo_view.cpp
        SOURCES convo_list_model.h
        SOURCES convo_list_model.cpp
        SOURCES chat.h
        SOURCES chat.cpp
        QML_FILES ConvoListView.qml
        QML_FILES ConvoViewDelegate.qml
        SOURCES message_view.h
        SOURCES message_view.cpp
        SOURCES message_list_model.h
        SOURCES message_list_model.cpp
        QML_FILES MessagesListView.qml
        QML_FILES MessagesListHeader.qml
        QML_FILES MessageViewDelegate.qml
        QML_FILES StartConversation.qml
        SOURCES profile_matcher.h
        QML_FILES QuoteLabeler.qml
        SOURCES focus_hashtags.h
        SOURCES focus_hashtags.cpp
        QML_FILES FocusHashtags.qml
        QML_FILES AddFocusHashtag.qml
        QML_FILES DatePicker.qml
        QML_FILES EditComboBox.qml
        QML_FILES SimpleAuthorListPage.qml
        SOURCES starter_pack.h
        SOURCES starter_pack.cpp
        SOURCES starter_pack_list_model.h
        SOURCES starter_pack_list_model.cpp
        QML_FILES
        QML_FILES StarterPackViewDelegate.qml
        QML_FILES StarterPackView.qml
        QML_FILES SkyListView.qml
        QML_FILES EditThreadPrefix.qml
        SOURCES meme_maker.h
        SOURCES meme_maker.cpp
        QML_FILES MemeEditor.qml
        QML_FILES GifView.qml
        QML_FILES MenuWithInfoItem.qml
        QML_FILES BlinkingOpacity.qml
        QML_FILES QuoteStarterPack.qml
        QML_FILES StatQuotes.qml
        QML_FILES QuotePostFeedView.qml
        SOURCES postgate.h
        SOURCES video_view.h
        QML_FILES VideoView.qml
        QML_FILES SkyPage.qml
        QML_FILES VideoAttachment.qml
        QML_FILES VideoThumbnail.qml
        QML_FILES RoundCornerMask.qml
        QML_FILES FullVideoView.qml
        SOURCES temp_file_holder.h
        SOURCES temp_file_holder.cpp
        SOURCES video_upload_limits.h
        SOURCES video_utils.h
        SOURCES video_utils.cpp
        QML_FILES VideoEditor.qml
        SOURCES m3u8_parser.h
        SOURCES m3u8_parse.cpp
        SOURCES m3u8_reader.h
        SOURCES m3u8_reader.cpp
        QML_FILES VideoUploadLimits.qml
        SOURCES network_utils.h
        SOURCES network_utils.cpp
        SOURCES video_encoder.h
        SOURCES video_encoder.cpp
        SOURCES gif_to_video_converter.h
        SOURCES gif_to_video_converter.cpp
        QML_FILES ProgressDialog.qml
        QML_FILES ConvertGifDialog.qml
        QML_FILES SkyCleanedTextLine.qml
        QML_FILES SkyFooterButton.qml
        SOURCES post_filter.h
        SOURCES post_filter.cpp
        SOURCES filtered_post_feed_model.h
        SOURCES filtered_post_feed_model.cpp
        QML_FILES TimelinePage.qml
)

if (NOT ANDROID)
    set(COVERAGE_LIB -lgcov)
endif()

target_link_libraries(libskywalker
    PRIVATE libatproto
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Svg
    ${COVERAGE_LIB}
)

target_include_directories(libskywalker INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ..)
