cmake_minimum_required(VERSION 3.25)

project(hands-free-chess-clock LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(WHISPER_CUBLAS 1)

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

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

target_sources(${PROJECT_NAME} PUBLIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
  ${ALSA_LIBRARIES}
  ${OpenCV_LIBS}
  whisper
)
