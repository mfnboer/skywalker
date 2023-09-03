cmake_minimum_required(VERSION 3.16)

project(appskywalker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup()

add_subdirectory(atproto/lib)

qt_add_executable(appskywalker
    main.cpp
)

qt_add_qml_module(appskywalker
    URI skywalker
    VERSION 1.0
    QML_FILES
        Main.qml
        Login.qml
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
    QML_FILES
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
    RESOURCES
        qtquickcontrols2.conf
        SOURCES svg_image.h
        QML_FILES StatIcon.qml
)

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
)

target_link_libraries(appskywalker
    PRIVATE libatproto
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appskywalker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
