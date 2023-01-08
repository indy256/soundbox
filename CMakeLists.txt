cmake_minimum_required(VERSION 3.16)
project(soundbox LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_definitions(-DAL_LIBTYPE_STATIC)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

qt_add_executable(soundbox
    sdl.h
    main.cpp
    window.cpp window.h
)

set_target_properties(soundbox PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include_directories(${CMAKE_SOURCE_DIR}/submodules/openal-soft/include)
include_directories(${CMAKE_SOURCE_DIR}/submodules/FFmpeg)
include_directories(${CMAKE_SOURCE_DIR}/submodules/SDL/include)

IF (WIN32)
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        winmm.lib
        bcrypt.lib
        imm32.lib
        Setupapi.lib
        Version.lib
        ${CMAKE_SOURCE_DIR}/submodules/openal-soft/build/Debug/OpenAL32.lib
        D:/projects/playsound/submodules/SDL/SDL-build/Release/SDL3-static.lib
        D:/projects/playsound/submodules/FFmpeg/libavcodec/libavcodec.a
        D:/projects/playsound/submodules/FFmpeg/libavformat/libavformat.a
        D:/projects/playsound/submodules/FFmpeg/libavutil/libavutil.a
        D:/projects/playsound/submodules/FFmpeg/libswresample/libswresample.a
    )
ELSEIF (APPLE)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_UNIT AudioUnit)
    find_library(OPENAL openal)
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${AUDIO_TOOLBOX}
        ${CORE_AUDIO}
        ${AUDIO_UNIT}
        ${OPENAL}
    )
ELSE()
    find_library(OPENAL openal)
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${OPENAL}
    )
ENDIF()
