cmake_minimum_required(VERSION 3.16)

project(test_atproto LANGUAGES CXX)

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
    main.cpp
    test_search_utils.h
)

target_link_libraries(test_skywalker
    PRIVATE libskywalker
    PRIVATE Qt6::Test
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

