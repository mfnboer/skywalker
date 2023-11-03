cmake_minimum_required(VERSION 3.16)

project(appskywalker VERSION 0.1 LANGUAGES CXX)
set(VERSION_CODE 1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Quick QuickControls2)

qt_standard_project_setup()

add_subdirectory(atproto/lib)

add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_compile_options(-Wall -Wextra -Werror)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

qt_add_executable(appskywalker
    main.cpp
)

qt_add_qml_module(appskywalker
    URI skywalker
    VERSION ${PROJECT_VERSION}
    NO_RESOURCE_TARGET_PATH
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
        qtquickcontrols2.conf
        android/src/com/gmail/mfnboer/QPhotoPicker.java
        android/src/com/gmail/mfnboer/FileUtils.java
        android/src/com/gmail/mfnboer/GMSEmojiFontDownloader.java
        fonts/NotoSansMath-Regular.ttf
        QML_FILES ContentLabels.qml
        RESOURCES android/src/com/gmail/mfnboer/ShareUtils.java
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
        RESOURCES android/src/com/gmail/mfnboer/PasswordStorageHelper.java
        SOURCES password_encryption.h
        SOURCES password_encryption.cpp
        SOURCES user_settings.h
        SOURCES user_settings.cpp
        RESOURCES android/src/com/gmail/mfnboer/SkywalkerActivity.java
        QML_FILES SkyTextInput.qml
        QML_FILES SignIn.qml
        RESOURCES images/skywalker.png
        QML_FILES SelectSignInUser.qml
        QML_FILES SwitchUserDrawer.qml
        QML_FILES AltTextEditor.qml
        QML_FILES MenuItemSvg.qml


)

set_target_properties(appskywalker PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    QT_ANDROID_TARGET_SDK_VERSION 33
    QT_ANDROID_VERSION_NAME ${PROJECT_VERSION}
    QT_ANDROID_VERSION_CODE ${VERSION_CODE})


if (ANDROID)
    FetchContent_Declare(
      android_openssl
      DOWNLOAD_EXTRACT_TIMESTAMP true
      URL      https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
#      URL_HASH MD5=c97d6ad774fab16be63b0ab40f78d945 #optional
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
endif()

if (ANDROID)
    add_android_openssl_libraries(appskywalker)
endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appskywalker PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appskywalker
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_ABIS "arm64-v8a;armeabi-v7a"
)

target_link_libraries(appskywalker
    PRIVATE libatproto
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appskywalker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
