set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Svg)

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID AND NOT CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
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
        qml/Main.qml
        qml/Login.qml
        qml/Avatar.qml
        qml/ImagePreview1.qml
        qml/ImagePreview2.qml
        qml/ImagePreview3.qml
        qml/ImagePreview4.qml
        qml/ExternalView.qml
        qml/RecordView.qml
        qml/PostHeader.qml
        qml/PostBody.qml
        qml/RecordWithMediaView.qml
        qml/RoundedFrame.qml
        qml/ThumbImageView.qml
        qml/FullImageView.qml
        qml/SkySvg.qml
        qml/StatIcon.qml
        qml/StatusPopup.qml
        qml/TimelineView.qml
        qml/SvgButton.qml
        qml/PostFeedViewDelegate.qml
        qml/PostThreadView.qml
        qml/ComposePost.qml
        qml/GuiSettings.qml
        qml/LinkCardView.qml
        qml/QuotePost.qml
        qml/NotificationViewDelegate.qml
        qml/NotificationListView.qml
        qml/PostStats.qml
        qml/SkyFooter.qml
        qml/AuthorView.qml
        qml/AuthorViewDelegate.qml
        qml/AuthorListView.qml
        qml/SkyLabel.qml
        qml/SkyButton.qml
        qml/Message.qml
        qml/PostButton.qml
    RESOURCES
        QML_FILES qml/ContentLabels.qml
        QML_FILES qml/ReplyToRow.qml
        SOURCES content_filter.h
        SOURCES content_filter.cpp
        QML_FILES qml/FilteredImageWarning.qml
        SOURCES link_utils.h
        SOURCES link_utils.cpp
        SOURCES edit_user_preferences.h
        SOURCES content_group_list_model.h
        SOURCES content_group_list_model.cpp
        SOURCES edit_user_preferences.cpp
        QML_FILES qml/SettingsDrawer.qml
        QML_FILES qml/SkyMenuItem.qml
        QML_FILES qml/SettingsForm.qml
        QML_FILES qml/ContentFilterSettings.qml
        SOURCES enums.cpp
        QML_FILES qml/SkyRadioButton.qml
        SOURCES search_utils.h
        SOURCES search_utils.cpp
        QML_FILES qml/SimpleAuthorListView.qml
        QML_FILES qml/SearchView.qml
        SOURCES search_post_feed_model.h
        SOURCES search_post_feed_model.cpp
        QML_FILES qml/ImageAutoRetry.qml
        SOURCES facet_highlighter.h
        SOURCES facet_highlighter.cpp
        SOURCES wrapped_skywalker.h
        SOURCES wrapped_skywalker.cpp
        SOURCES password_encryption.h
        SOURCES password_encryption.cpp
        SOURCES user_settings.h
        SOURCES user_settings.cpp
        QML_FILES qml/SkyTextInput.qml
        QML_FILES qml/SignIn.qml
        QML_FILES qml/SelectSignInUser.qml
        QML_FILES qml/SwitchUserDrawer.qml
        QML_FILES qml/AltTextEditor.qml
        QML_FILES qml/MenuItemSvg.qml
        QML_FILES qml/Bookmarks.qml
        QML_FILES qml/About.qml
        QML_FILES qml/StartupStatus.qml
        QML_FILES qml/SimpleHeader.qml
        SOURCES shared_image_provider.h
        SOURCES shared_image_provider.cpp
        QML_FILES qml/FlickableRefresher.qml
        SOURCES definitions.h
        QML_FILES qml/Report.qml
        SOURCES report_reason.h
        SOURCES report_utils.h
        SOURCES report_utils.cpp
        QML_FILES qml/SkyTextEdit.qml
        QML_FILES qml/SimpleButtonHeader.qml
        QML_FILES qml/ReportDetailsEditor.qml
        SOURCES muted_words.h
        SOURCES muted_words.cpp
        QML_FILES qml/MutedWords.qml
        QML_FILES qml/AddMutedWord.qml
        SOURCES normalized_word_index.h
        SOURCES normalized_word_index.cpp
        SOURCES tenor.h
        SOURCES tenor_gif.h
        SOURCES tenor_category.h
        SOURCES tenor.cpp
        QML_FILES qml/SearchHeader.qml
        QML_FILES qml/TenorSearch.qml
        SOURCES tenor_preview_row.h
        SOURCES tenor_preview_row.cpp
        SOURCES tenor_gif_overview_model.h
        SOURCES tenor_gif_overview_model.cpp
        QML_FILES qml/AnimatedImageAutoRetry.qml
        QML_FILES qml/ThumbAnimatedImageView.qml
        QML_FILES qml/AnimatedImagePreview.qml
        QML_FILES qml/FullAnimatedImageView.qml
        QML_FILES qml/AddReplyRestrictions.qml
        SOURCES feed_list_model.h
        SOURCES generator_view.h
        SOURCES generator_view.cpp
        SOURCES feed_list_model.cpp
        QML_FILES qml/SearchFeeds.qml
        QML_FILES qml/GeneratorViewDelegate.qml
        QML_FILES qml/PostFeedView.qml
        QML_FILES qml/FeedAvatar.qml
        QML_FILES qml/ExpandFeedsButton.qml
        SOURCES favorite_feeds.h
        SOURCES favorite_feeds.cpp
        SOURCES local_feed_model_changes.h
        SOURCES local_feed_model_changes.cpp
        SOURCES feed_utils.h
        SOURCES feed_utils.cpp
        QML_FILES qml/FeedDescriptionView.qml
        QML_FILES qml/QuoteFeed.qml
        QML_FILES qml/PostFeedHeader.qml
        QML_FILES qml/ListListView.qml
        QML_FILES qml/SimpleDescriptionHeader.qml
        SOURCES list_list_model.h
        SOURCES list_view.h
        SOURCES list_view.cpp
        SOURCES list_list_model.cpp
        QML_FILES qml/ListViewDelegate.qml
        QML_FILES qml/ListAvatar.qml
        QML_FILES qml/EmptyListIndication.qml
        QML_FILES qml/EditList.qml
        QML_FILES qml/SkyFormattedTextEdit.qml
        QML_FILES qml/AuthorTypeaheadView.qml
        QML_FILES qml/TextLengthBar.qml
        QML_FILES qml/TextLengthCounter.qml
        QML_FILES qml/EditAvatar.qml
        QML_FILES qml/ImageFileDialog.qml
        QML_FILES qml/ListFeedDescriptionView.qml
        QML_FILES qml/QuoteList.qml
        QML_FILES qml/SearchAuthor.qml
        SOURCES profile_utils.h
        SOURCES profile_utils.cpp
        SOURCES favorite_feed_view.h
        SOURCES favorite_feed_view.cpp
        QML_FILES qml/UserListsPage.qml
        QML_FILES qml/ModerationListsPage.qml
        SOURCES local_list_model_changes.h
        SOURCES local_list_model_changes.cpp
        QML_FILES qml/ListViewerState.qml
        QML_FILES qml/FavoriteStar.qml
        SOURCES list_view_include.h
        QML_FILES qml/AddUserListListView.qml
        QML_FILES qml/AddUserListViewDelegate.qml
        SOURCES gif_utils.h
        SOURCES gif_utils.cpp
        QML_FILES qml/AuthorPostsList.qml
        QML_FILES qml/EditProfile.qml
        SOURCES local_profile_changes.h
        SOURCES local_profile_changes.cpp
        SOURCES unicode_fonts.h
        SOURCES unicode_fonts.cpp
        QML_FILES qml/PagingComboBox.qml
        QML_FILES qml/AddContentWarning.qml
        SOURCES content_label.h
        QML_FILES qml/ContentLabelInfo.qml
        QML_FILES qml/AccessibilityUtils.qml
        QML_FILES qml/AccessibleMenuItem.qml
        QML_FILES qml/AccessibleTabButton.qml
        QML_FILES qml/CloseMenuItem.qml
        QML_FILES qml/AccessibleCheckBox.qml
        QML_FILES qml/AccessibleText.qml
        QML_FILES qml/AccessibleSwitch.qml
        SOURCES emoji_fix_highlighter.h
        SOURCES emoji_fix_highlighter.cpp
        QML_FILES qml/SkyCleanedText.qml
        SOURCES grapheme_info.h
        SOURCES grapheme_info.cpp
        SOURCES file_utils.h
        SOURCES file_utils.cpp
        SOURCES draft_posts.h
        SOURCES draft_posts.cpp
        SOURCES draft_posts_model.h
        SOURCES draft_posts_model.cpp
        QML_FILES qml/DraftPostsView.qml
        QML_FILES qml/DraftPostViewDelegate.qml
        SOURCES draft_post_data.h
        SOURCES atproto_image_provider.h
        SOURCES atproto_image_provider.cpp
        SOURCES hashtag_index.h
        SOURCES hashtag_index.cpp
        QML_FILES qml/HashtagTypeaheadView.qml
        QML_FILES qml/HashtagListView.qml
        QML_FILES qml/SearchPostScope.qml
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
        QML_FILES qml/FontComboBox.qml
        QML_FILES qml/AddGifButton.qml
        QML_FILES qml/ImageScroller.qml
        QML_FILES qml/SvgTransparentButton.qml
        QML_FILES qml/ComposePostItem.qml
        QML_FILES qml/SeparatorLine.qml
        SOURCES tenor_gif.cpp
        QML_FILES qml/LanguageComboBox.qml
        SOURCES language_utils.h
        SOURCES language_utils.cpp
        QML_FILES qml/LanguageComboCheckBox.qml
        QML_FILES qml/LanguageLabels.qml
        QML_FILES qml/ModeratorIcon.qml
        SOURCES labeler.h
        SOURCES labeler.cpp
        QML_FILES qml/ContentGroupDelegate.qml
        SOURCES content_group.h
        SOURCES content_group.cpp
        QML_FILES qml/StatAuthors.qml
        QML_FILES qml/HeaderText.qml
        QML_FILES qml/ReportAppeal.qml
        SOURCES utils.h
        SOURCES utils.cpp
        QML_FILES qml/AuthorComboBox.qml
        QML_FILES qml/AuthorItemDelegate.qml
        SOURCES content_label.cpp
        SOURCES anniversary_card.h
        SOURCES anniversary_card.cpp
        QML_FILES qml/AnniversaryCardMaker.qml
        QML_FILES qml/ColorSelector.qml
        SOURCES anniversary.h
        SOURCES anniversary.cpp
        QML_FILES qml/BadgeCounter.qml
        SOURCES chat_profile.h
        SOURCES chat_profile.cpp
        SOURCES convo_view.h
        SOURCES convo_view.cpp
        SOURCES convo_list_model.h
        SOURCES convo_list_model.cpp
        SOURCES chat.h
        SOURCES chat.cpp
        QML_FILES qml/ConvoListView.qml
        QML_FILES qml/ConvoViewDelegate.qml
        SOURCES message_view.h
        SOURCES message_view.cpp
        SOURCES message_list_model.h
        SOURCES message_list_model.cpp
        QML_FILES qml/MessagesListView.qml
        QML_FILES qml/MessagesListHeader.qml
        QML_FILES qml/MessageViewDelegate.qml
        QML_FILES qml/StartConversation.qml
        SOURCES profile_matcher.h
        QML_FILES qml/QuoteLabeler.qml
        SOURCES focus_hashtags.h
        SOURCES focus_hashtags.cpp
        QML_FILES qml/FocusHashtags.qml
        QML_FILES qml/AddFocusHashtag.qml
        QML_FILES qml/DatePicker.qml
        QML_FILES qml/EditComboBox.qml
        QML_FILES qml/SimpleAuthorListPage.qml
        SOURCES starter_pack.h
        SOURCES starter_pack.cpp
        SOURCES starter_pack_list_model.h
        SOURCES starter_pack_list_model.cpp
        QML_FILES qml/StarterPackViewDelegate.qml
        QML_FILES qml/StarterPackView.qml
        QML_FILES qml/SkyListView.qml
        QML_FILES qml/EditThreadPrefix.qml
        SOURCES meme_maker.h
        SOURCES meme_maker.cpp
        QML_FILES qml/MemeEditor.qml
        QML_FILES qml/GifView.qml
        QML_FILES qml/MenuWithInfoItem.qml
        QML_FILES qml/BlinkingOpacity.qml
        QML_FILES qml/QuoteStarterPack.qml
        QML_FILES qml/StatQuotes.qml
        QML_FILES qml/QuotePostFeedView.qml
        SOURCES postgate.h
        SOURCES video_view.h
        QML_FILES qml/VideoView.qml
        QML_FILES qml/SkyPage.qml
        QML_FILES qml/VideoAttachment.qml
        QML_FILES qml/VideoThumbnail.qml
        QML_FILES qml/RoundCornerMask.qml
        QML_FILES qml/FullVideoView.qml
        SOURCES temp_file_holder.h
        SOURCES temp_file_holder.cpp
        SOURCES video_upload_limits.h
        SOURCES video_utils.h
        SOURCES video_utils.cpp
        QML_FILES qml/VideoEditor.qml
        SOURCES m3u8_parser.h
        SOURCES m3u8_parse.cpp
        SOURCES m3u8_reader.h
        SOURCES m3u8_reader.cpp
        QML_FILES qml/VideoUploadLimits.qml
        SOURCES network_utils.h
        SOURCES network_utils.cpp
        SOURCES video_encoder.h
        SOURCES video_encoder.cpp
        SOURCES gif_to_video_converter.h
        SOURCES gif_to_video_converter.cpp
        QML_FILES qml/ProgressDialog.qml
        QML_FILES qml/ConvertGifDialog.qml
        QML_FILES qml/SkyCleanedTextLine.qml
        QML_FILES qml/SkyFooterButton.qml
        SOURCES post_filter.h
        SOURCES post_filter.cpp
        SOURCES filtered_post_feed_model.h
        SOURCES filtered_post_feed_model.cpp
        QML_FILES qml/TimelinePage.qml
        QML_FILES qml/AddUserTimelineView.qml
        QML_FILES qml/SkyTabWithCloseButton.qml
        QML_FILES qml/AddFocusHashtagTimelineView.qml
        QML_FILES qml/AddHashtagTimelineView.qml
        SOURCES image_utils.h
        SOURCES image_utils.cpp
        QML_FILES qml/ScriptRecognitionMenuItem.qml
        QML_FILES qml/SettingsAccount.qml
        QML_FILES qml/SettingsHomeFeed.qml
        QML_FILES qml/SettingsChat.qml
        QML_FILES qml/SettingsLanguage.qml
        QML_FILES qml/SettingsAppearance.qml
        QML_FILES qml/SettingsNotifications.qml
        SOURCES trending_topic.h
        SOURCES trending_topic.cpp
        SOURCES trending_topic_list_model.h
        SOURCES trending_topic_list_model.cpp
        QML_FILES qml/SettingsSearch.qml
        SOURCES search_feed.h
        SOURCES search_feed.cpp
        QML_FILES qml/SearchFeedView.qml
        QML_FILES qml/VirtualKeyboardHandler.qml
        QML_FILES qml/MediaFeedViewDelegate.qml
        QML_FILES qml/MediaFeedView.qml
        SOURCES video_cache.h
        SOURCES video_cache.cpp
        SOURCES signal_object.h
        SOURCES signal_object.cpp
        SOURCES feed_pager.h
        SOURCES list_store.h
        SOURCES list_store.cpp
        SOURCES graph_listener.h
        SOURCES graph_listener.cpp
        QML_FILES qml/PostHeaderWithAvatar.qml
        QML_FILES qml/ImageWithZoom.qml
        QML_FILES qml/RewindStatus.qml
        SOURCES post_interaction_settings.h
        SOURCES post_interaction_settings.cpp
        SOURCES list_cache.h
        SOURCES list_cache.cpp
        SOURCES display_utils.h
        SOURCES scoped_handle.h
        SOURCES display_utils.cpp
        QML_FILES qml/ThumbImageKnownSizeView.qml
        QML_FILES qml/ThumbImageUnknownSizeView.qml
        QML_FILES qml/MediaTilesFeedViewDelegate.qml
        QML_FILES qml/MediaTilesFeedView.qml
        SOURCES virtual_keyboard_utils.h
        SOURCES virtual_keyboard_utils.cpp
        SOURCES base_list_model.h
        QML_FILES qml/FavoritesTabBar.qml
        QML_FILES qml/SkyTabBar.qml
        QML_FILES qml/FavoriteTabButton.qml
        QML_FILES qml/FavoritesSwipeView.qml
        QML_FILES qml/FavoritesSorter.qml
        QML_FILES qml/SkyRoundRadioButton.qml
        QML_FILES qml/SvgPlainButton.qml
        QML_FILES qml/SkyRadioMenuItem.qml
        QML_FILES qml/DragDropRectangle.qml
        QML_FILES qml/TimelineViewsSorter.qml
        QML_FILES qml/SkySettingsTabButton.qml
        QML_FILES qml/DurationLabel.qml
        SOURCES emoji_names.cpp
        SOURCES emoji_names.h
        QML_FILES qml/EmojiNamesList.qml
        QML_FILES qml/KnownFollowers.qml
        QML_FILES qml/ConvoRequestButtonRow.qml
        QML_FILES qml/FeedViewerState.qml
        QML_FILES qml/UnknownEmbedView.qml
        SOURCES web_link.h
        SOURCES web_link.cpp
        SOURCES text_differ.h
        SOURCES text_differ.cpp
        QML_FILES qml/EditEmbeddedLink.qml
        SOURCES facet_utils.h
        SOURCES facet_utils.cpp
        SOURCES split_text_boundary_finder.h
        SOURCES split_text_boundary_finder.cpp
        SOURCES text_splitter.h
        SOURCES text_splitter.cpp
        QML_FILES qml/LinkCatcher.qml
        QML_FILES qml/HashtagContextMenu.qml
        SOURCES post_thread_cache.h
        SOURCES post_thread_cache.cpp
        QML_FILES qml/ReactionsMenu.qml
        QML_FILES qml/ReactionButton.qml
        SOURCES reaction_view.h
        SOURCES reaction_view.cpp
        SOURCES message_and_reaction_view.h
        SOURCES message_and_reaction_view.cpp
        QML_FILES qml/ReactionListView.qml
        QML_FILES qml/ReactionViewDelegate.qml
        QML_FILES qml/AnimateToFullImage.qml
        QML_FILES qml/ImageAltText.qml
        QML_FILES qml/FullImageViewLoader.qml
        QML_FILES qml/SideBar.qml
        QML_FILES qml/VerifiedBadge.qml
        QML_FILES qml/VerifierBadge.qml
        QML_FILES qml/VerifiedDialog.qml
        QML_FILES qml/AuthorNameAndStatus.qml
        QML_FILES qml/TrustedVerifierDialog.qml
        QML_FILES qml/AuthorNameAndStatusMultiLine.qml
        QML_FILES qml/PlaceholderHeader.qml
        QML_FILES qml/SwipeListView.qml
        SOURCES songlink_links.h
        SOURCES songlink_links.cpp
        SOURCES songlink.h
        SOURCES songlink.cpp
        QML_FILES qml/SonglinkWidget.qml
        SOURCES songlink_cache.h
        SOURCES songlink_cache.cpp
        SOURCES uri_with_expiry.h
        SOURCES uri_with_expiry.cpp
        QML_FILES qml/DurationInput.qml
        QML_FILES qml/BlockMuteUser.qml
        QML_FILES qml/AuthorViewerState.qml
        QML_FILES qml/LiveLabel.qml
        QML_FILES qml/ThumbImageFixedSizeView.qml
        QML_FILES qml/DeadFooterMargin.qml
        QML_FILES qml/DeadHeaderMargin.qml
        QML_FILES qml/NotificationTypeSetting.qml
        QML_FILES qml/NotificationIncludeSetting.qml
        SOURCES edit_notification_preferences.h
        SOURCES edit_notification_preferences.cpp
        SOURCES edit_notification_filterable_pref.h
        SOURCES edit_notification_filterable_pref.cpp
        SOURCES notification_utils.h
        SOURCES notification_utils.cpp
        SOURCES edit_notification_pref.h
        SOURCES edit_notification_pref.cpp
        QML_FILES qml/SubscribeActivity.qml
        QML_FILES qml/SkyMenu.qml
        SOURCES follows_activity_store.h
        SOURCES activity_status.h
        SOURCES follows_activity_store.cpp
        SOURCES activity_status.cpp
        QML_FILES qml/EditImage.qml
        SOURCES bookmarks_model.h
        SOURCES bookmarks_model.cpp
        SOURCES bookmarks.h
        SOURCES bookmarks.cpp
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
