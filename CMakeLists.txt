cmake_minimum_required(VERSION 3.16)

project(skywalker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    RESOURCES
        qtquickcontrols2.conf
        SOURCES post_feed_model.h
        SOURCES post_feed_model.cpp
        SOURCES post.h
        SOURCES profile.h
        SOURCES post.cpp
        SOURCES profile.cpp
        QML_FILES Avatar.qml
        SOURCES image_view.h
        QML_FILES ImagePreview1.qml
        QML_FILES ImagePreview2.qml
        QML_FILES ImagePreview3.qml
        QML_FILES ImagePreview4.qml
        QML_FILES
        SOURCES external_view.h
        QML_FILES ExternalView.qml
        SOURCES record_view.h
        SOURCES record_view.cpp
        QML_FILES RecordView.qml
        QML_FILES PostHeader.qml
        RESOURCES
        QML_FILES PostBody.qml
        SOURCES record_with_media_view.h
        SOURCES record_with_media_view.cpp
        QML_FILES RecordWithMediaView.qml
        SOURCES external_view.cpp
        QML_FILES RoundedFrame.qml
)

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
