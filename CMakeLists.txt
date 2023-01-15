cmake_minimum_required(VERSION 3.16)
project(soundbox LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_definitions(-DAL_LIBTYPE_STATIC)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

qt_add_executable(soundbox
    src/sdl.h
    src/main.cpp
    src/window.cpp src/window.h
)

set_target_properties(soundbox PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

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
        ${CMAKE_SOURCE_DIR}/submodules/SDL/SDL-build/Release/SDL3-static.lib
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavcodec/libavcodec.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavformat/libavformat.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavutil/libavutil.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libswresample/libswresample.a
    )
ELSEIF (APPLE)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_UNIT AudioUnit)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(VIDEO_TOOLBOX VideoToolbox)
    find_library(CORE_GRAPHICS CoreGraphics)
    find_library(CORE_MEDIA CoreMedia)
    find_library(CORE_VIDEO CoreVideo)
    find_library(FORCE_FEEDBACK ForceFeedback)
    find_library(GAME_CONTROLLER GameController)
    find_library(IO_KIT IOKit)
    find_library(CORE_HAPTICS CoreHaptics)        
    find_library(CORE_SERVICES CoreServices)
    find_library(METAL Metal)
    find_library(QUARTZ_CORE QuartzCore)
    find_library(APP_KIT AppKit)
    find_library(CARBON Carbon)
    find_library(ICONV NAMES iconv)
    find_library(ZLIB NAMES z)

    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${AUDIO_TOOLBOX}
        ${CORE_AUDIO}
        ${AUDIO_UNIT}
        ${CORE_FOUNDATION}
        ${VIDEO_TOOLBOX}
        ${CORE_GRAPHICS}
        ${CORE_MEDIA}
        ${CORE_VIDEO}
        ${FORCE_FEEDBACK}
        ${GAME_CONTROLLER}
        ${IO_KIT}
        ${CORE_HAPTICS}
        ${CORE_SERVICES}
        ${METAL}
        ${QUARTZ_CORE}
        ${APP_KIT}
        ${CARBON}
        ${ICONV}
        ${ZLIB}
        ${CMAKE_SOURCE_DIR}/submodules/SDL/build/libSDL3.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavformat/libavformat.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavcodec/libavcodec.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavutil/libavutil.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libswresample/libswresample.a
    )
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -lpthread -ldl -lX11 -lvdpau -lva -lva-drm")
    target_link_libraries(soundbox PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${CMAKE_SOURCE_DIR}/submodules/SDL/build/libSDL3.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavformat/libavformat.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavcodec/libavcodec.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libavutil/libavutil.a
        ${CMAKE_SOURCE_DIR}/submodules/FFmpeg/libswresample/libswresample.a
    )
ENDIF()
