cmake_minimum_required(VERSION 3.16)
project(soundbox LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_definitions(-DAL_LIBTYPE_STATIC)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

qt_add_executable(soundbox
    sound.h
    main.cpp
    window.cpp window.h
)

set_target_properties(soundbox PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include_directories(${CMAKE_SOURCE_DIR}/submodules/openal-soft/include)

IF (WIN32)
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        winmm.lib
        ${CMAKE_SOURCE_DIR}/submodules/openal-soft/build/Release/OpenAL32.lib
    )
ELSEIF (APPLE)
ELSE()
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        /usr/local/lib/libopenal.a
    )
ENDIF()
