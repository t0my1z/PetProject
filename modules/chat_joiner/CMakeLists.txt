cmake_minimum_required(VERSION 3.23)
project(chat_joiner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get the comms_manager include directory (parent project)
set(COMMS_MANAGER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../comms_manager/include")

# Create the module as a shared library
add_library(chat_joiner SHARED)

target_sources(chat_joiner
    PRIVATE
        ChatJoiner.cpp
    PUBLIC
        ChatJoiner.hpp
)

target_include_directories(chat_joiner
    PRIVATE
        "${COMMS_MANAGER_INCLUDE_DIR}"
)

# Set output directory for the module library
set_target_properties(chat_joiner PROPERTIES
    PREFIX "lib"
    SUFFIX ".so"
)

if(WIN32)
    set_target_properties(chat_joiner PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

install(
    TARGETS chat_joiner
    LIBRARY DESTINATION ${INSTALL_DIR}/lib/modules/chat_joiner
    RUNTIME DESTINATION ${INSTALL_DIR}/lib/modules/chat_joiner
)
