cmake_minimum_required(VERSION 3.0)
project(soundtracker)
if (WIN32)
  add_subdirectory(SDL)
  set(HAVE_SDL2 SDL2-static)
else (WIN32)
  find_library(HAVE_SDL2 SDL2)
endif (WIN32)
if (APPLE)
  find_library(HAVE_APPKIT AppKit PATHS ${CMAKE_OSX_SYSROOT}/System/Library PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)
else (APPLE)
  find_library(HAVE_JACK jack)
endif (APPLE)
add_executable(ssinter src/soundchip.cpp src/ssinter.cpp src/ssmain.cpp)
add_executable(ssbench src/soundchip.cpp src/ssinter.cpp src/ssbench.cpp)
set(TRACKER_SRC src/blip_buf.c
                src/unifontfull.cpp
                src/soundchip.cpp
                src/song.cpp
                src/player.cpp
                src/macroStatus.cpp
                src/hlesoundchip.cpp
                src/graphics.cpp
                src/ssinter.cpp
                src/fextra.cpp
                src/main.cpp)
if (APPLE)
list(APPEND TRACKER_SRC src/nsstub.m)
endif (APPLE)
if (WIN32)
  add_executable(soundtracker WIN32 ${TRACKER_SRC})
else()
  add_executable(soundtracker ${TRACKER_SRC})
endif()
target_link_libraries(ssinter ${HAVE_SDL2})
target_link_libraries(soundtracker ${HAVE_SDL2})
if (WIN32)
  target_link_libraries(ssinter SDL2main)
  target_link_libraries(soundtracker SDL2main)
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
