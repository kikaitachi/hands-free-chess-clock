cmake_minimum_required(VERSION 3.28)

project(hands-free-chess-clock LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

find_package(ALSA REQUIRED)
include_directories(${ALSA_INCLUDE_DIRS})
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(whisper.cpp
  GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
  GIT_TAG v1.5.4
)
FetchContent_MakeAvailable(whisper.cpp)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    src/main.cpp
    src/audio_capture.cpp
    src/command_parser.cpp
    src/display.cpp
    src/logger.cpp
    src/mpscq.cpp
    src/speech_to_text.cpp
    src/video_capture.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${ALSA_LIBRARIES}
  ${OpenCV_LIBS}
  whisper
)
