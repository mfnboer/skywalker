cmake_minimum_required(VERSION 3.16)

project(test_skywalker LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_compile_options(-Wall -Wextra -Werror)

if (NOT ANDROID)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

qt_add_executable(test_skywalker
    test_hashtag_index.h
    test_muted_words.h
    test_post_feed_model.h
    test_search_utils.h
    main.cpp
    test_unicode_fonts.h
    test_anniversary.h
    test_focus_hashtags.h
    test_filtered_post_feed_model.h
    test_text_differ.h
    test_text_splitter.h
    test_uri_with_expiry.h)

set(LINK_LIBS
    PRIVATE libatproto
    PRIVATE libskywalker
    PRIVATE Qt6::Test
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

target_link_libraries(test_skywalker ${LINK_LIBS})
