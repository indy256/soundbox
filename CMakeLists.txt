cmake_minimum_required(VERSION 3.16)
project(soundbox LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(soundbox
    main.cpp
    window.cpp window.h
)

set_target_properties(soundbox PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(soundbox PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
