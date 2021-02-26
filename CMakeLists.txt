cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
project(gesture-piano)

# This tells the compiler to not aggressively optimize and
# to include debugging information so that the debugger
# can properly read what's going on.
set(CMAKE_BUILD_TYPE Debug)

# Let's ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# Let's nicely support folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Warning flags
if(MSVC)
    # warning level 3 and all warnings as errors
    add_compile_options(/W2 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wpedantic -Werror)
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)

# Replace ENTER_OPEN_CV_BUILD_PATH_HERE with the path of your OpenCV install folder
set(OpenCV_DIR "ENTER_OPEN_CV_BUILD_PATH_HERE")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(include)
# add libs you need
set(OpenCV_LIBS opencv_core opencv_video opencv_imgproc opencv_highgui opencv_imgcodecs opencv_photo opencv_objdetect)

# FetchContent_MakeAvailable was not added until CMake 3.14
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.x

)

# Adds Catch2 testing library
FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
    FetchContent_Populate(catch2)
    add_library(catch2 INTERFACE)
    target_include_directories(catch2 INTERFACE ${catch2_SOURCE_DIR}/single_include)
endif()

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()



get_filename_component(CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE)
get_filename_component(APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE)


include("${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake")



list(APPEND GESTURE_SOURCE_FILES gesture_recognition/calibration.cc gesture_recognition/hand_extractor.cc gesture_recognition/final_project_app.cc gesture_recognition/hand_tracker.cc gesture_recognition/gesture_wrapper.cc)
list(APPEND PIANO_SOURCE_FILES pianoapp/piano_engine.cc)
list(APPEND TEST_FILES tests/test_hand_extractor.cc tests/test_hand_tracker.cc tests/test_piano_engine.cc
        )


ci_make_app(
        APP_NAME       gesture-piano
       CINDER_PATH     ${CINDER_PATH}
        SOURCES         apps/cinder_app_main.cc ${GESTURE_SOURCE_FILES} ${PIANO_SOURCE_FILES}
        INCLUDES        include
        LIBRARIES      ${OpenCV_LIBS} nlohmann_json::nlohmann_json
)

ci_make_app(
        APP_NAME        gesture-piano-test
        CINDER_PATH     ${CINDER_PATH}
        SOURCES         tests/test_main.cc ${TEST_FILES} ${GESTURE_SOURCE_FILES} ${PIANO_SOURCE_FILES}
        INCLUDES        include
        LIBRARIES       catch2 ${OpenCV_LIBS}
)


if(MSVC)
    set_property(TARGET gesture-piano-test APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
endif()

#
#
