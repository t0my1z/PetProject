cmake_minimum_required(VERSION 3.23)

project(comms_project VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(INSTALL_DIR "windows")
elseif(UNIX AND NOT APPLE)
    set(INSTALL_DIR "linux")
elseif(APPLE)
    set(INSTALL_DIR "macos")
endif()

add_subdirectory("comms_manager")

# Build modules
add_subdirectory("modules/chat_creator")
add_subdirectory("modules/chat_joiner")