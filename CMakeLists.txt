cmake_minimum_required(VERSION 3.0)
project(soundtracker)

set(CMAKE_CXX_STANDARD 11)

if (ANDROID)
  set(BUILD_GUI OFF)
else (ANDROID)
  set(BUILD_GUI ON)
endif (ANDROID)

if (WIN32)
  set(SDL2_LIB SDL2-static)
else (WIN32)
  set(SDL2_LIB SDL2)
endif (WIN32)

add_subdirectory(fmt)

include_directories(fmt)

if (BUILD_GUI)
  if (WIN32)
    set(BUILD_SHARED_LIBS OFF)
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
  endif (WIN32)
  add_subdirectory(SDL)
  include_directories(SDL/include)
endif (BUILD_GUI)
if (APPLE)
  find_library(HAVE_APPKIT AppKit PATHS ${CMAKE_OSX_SYSROOT}/System/Library PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)
else (APPLE)
  find_library(HAVE_JACK jack)
endif (APPLE)
add_executable(ssinter src/soundchip.cpp src/ssinter.cpp src/ssmain.cpp src/utfutils.cpp)
add_executable(ssbench src/soundchip.cpp src/ssinter.cpp src/ssbench.cpp)
set(GUI_SRC imgui/imgui.cpp
imgui/imgui_demo.cpp
imgui/imgui_draw.cpp
imgui/imgui_tables.cpp
imgui/imgui_widgets.cpp
imgui/backends/imgui_impl_sdlrenderer.cpp
imgui/backends/imgui_impl_sdl.cpp
igfd/ImGuiFileDialog.cpp
)
set(TRACKER_SRC src/blip_buf.c
src/soundchip.cpp
src/song.cpp
src/player.cpp
src/macroStatus.cpp
src/hlesoundchip.cpp
src/ssinter.cpp
src/fextra.cpp
src/utfutils.cpp
src/font_main.cpp
src/font_pat.cpp
src/main.cpp)
if (BUILD_GUI)
  list(APPEND TRACKER_SRC ${GUI_SRC})
endif (BUILD_GUI)
if (APPLE)
  list(APPEND TRACKER_SRC src/nsstub.m)
endif (APPLE)
if (WIN32)
  add_executable(soundtracker WIN32 ${TRACKER_SRC})
else()
  add_executable(soundtracker ${TRACKER_SRC})
endif()
target_link_libraries(ssinter ${SDL2_LIB})
if (BUILD_GUI)
  target_include_directories(soundtracker PRIVATE imgui imgui/backends igfd)
  target_compile_definitions(soundtracker PUBLIC HAVE_GUI)
  target_compile_definitions(ssinter PUBLIC HAVE_GUI)
endif (BUILD_GUI)
target_link_libraries(soundtracker ${SDL2_LIB} fmt)
if (WIN32)
  target_link_libraries(ssinter SDL2main)
  target_link_libraries(soundtracker SDL2main -static)
endif (WIN32)
if (HAVE_JACK)
  target_compile_definitions(soundtracker PUBLIC JACK)
  target_link_libraries(soundtracker jack)
  target_compile_definitions(ssinter PUBLIC JACK)
  target_link_libraries(ssinter jack)
endif ()
if (APPLE)
  target_link_libraries(soundtracker ${HAVE_APPKIT})
else ()
  if (NOT WIN32)
    target_link_libraries(soundtracker X11)
  endif (NOT WIN32)
endif ()
