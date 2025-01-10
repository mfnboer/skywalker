cmake_minimum_required(VERSION 3.22)

project(appskywalker VERSION 3.11 LANGUAGES CXX)
set(VERSION_CODE 59)
set(APP_VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Multimedia)

qt_standard_project_setup()

add_compile_definitions(APP_VERSION="${APP_VERSION}")
add_compile_definitions(TENOR_API_KEY="$ENV{TENOR_API_KEY}")

add_subdirectory(atproto/lib)
add_subdirectory(skywalker)

if (NOT ANDROID)
    add_subdirectory(test)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

qt_add_executable(appskywalker
    main.cpp
)

qt_add_qml_module(appskywalker
    URI app
    VERSION ${PROJECT_VERSION}
    NO_RESOURCE_TARGET_PATH
    RESOURCES
        qtquickcontrols2.conf
        fonts/unicode.impact.ttf
        fonts/NotoSansMath-Regular.ttf
        fonts/NotoColorEmoji.ttf
        images/skywalker.png
        images/PB_tenor_logo_blue_horizontal.svg
        images/via_tenor_logo_blue.svg
        images/giphy_logo.png
        images/buycoffee.png
        android/src/com/gmail/mfnboer/QPhotoPicker.java
        android/src/com/gmail/mfnboer/FileUtils.java
        android/src/com/gmail/mfnboer/GMSEmojiFontDownloader.java
        android/src/com/gmail/mfnboer/ShareUtils.java
        android/src/com/gmail/mfnboer/PasswordStorageHelper.java
        android/src/com/gmail/mfnboer/SkywalkerActivity.java
        RESOURCES android/src/com/gmail/mfnboer/FontUtils.java
        RESOURCES android/src/com/gmail/mfnboer/NewMessageChecker.java
        RESOURCES android/src/com/gmail/mfnboer/NewMessageNotifier.java
        RESOURCES android/src/com/gmail/mfnboer/SkywalkerApplication.java
        RESOURCES android/src/com/gmail/mfnboer/DirectMessageActivity.java
        RESOURCES android/src/com/gmail/mfnboer/VideoTranscoder.java
        RESOURCES android/src/com/gmail/mfnboer/NetworkUtils.java
        RESOURCES android/src/com/gmail/mfnboer/QVideoEncoder.java
        RESOURCES android/src/com/gmail/mfnboer/TextExtractor.java
        RESOURCES android/src/com/gmail/mfnboer/LanguageDetection.java
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

set_target_properties(appskywalker PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    QT_ANDROID_TARGET_SDK_VERSION 34
    QT_ANDROID_SDK_BUILD_TOOLS_REVISION 34.0.0
    QT_ANDROID_VERSION_NAME ${APP_VERSION}
    QT_ANDROID_VERSION_CODE ${VERSION_CODE})

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
    PRIVATE libskywalker
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Multimedia
)

include(GNUInstallDirs)
install(TARGETS appskywalker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
