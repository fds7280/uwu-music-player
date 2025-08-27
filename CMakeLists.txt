cmake_minimum_required(VERSION 3.10)
project(UwuMusicPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_check_modules(TAGLIB REQUIRED taglib)

# Find libraries
find_library(NCURSES_LIBRARY ncurses)
find_library(SNDFILE_LIBRARY sndfile)

# Source files
set(SOURCES
    src/main.cpp
    src/utils/utils.cpp
    src/art/ascii_art.cpp
    src/metadata/metadata_reader.cpp
    src/audio/audio_player.cpp
    src/streaming/youtube_stream.cpp
    src/ui/ui_common.cpp
    src/ui/file_browser.cpp
    src/ui/playback_ui.cpp
    src/ui/search_ui.cpp
    src/playlist/playlist_manager.cpp
)

# Create executable
add_executable(uwu ${SOURCES})

# Include directories - ADD src DIRECTORY HERE
target_include_directories(uwu PRIVATE
    ${CMAKE_SOURCE_DIR}/src  # This is the important addition!
    ${PIPEWIRE_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(uwu
    ${NCURSES_LIBRARY}
    ${SNDFILE_LIBRARY}
    ${PIPEWIRE_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    pthread
)

# Compile options
target_compile_options(uwu PRIVATE
    ${PIPEWIRE_CFLAGS_OTHER}
    ${TAGLIB_CFLAGS_OTHER}
)
