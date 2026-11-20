project(comms_manager
    VERSION 1.0
    LANGUAGES CXX
)

find_package(Threads REQUIRED)

add_subdirectory(thirdparty/yaml-cpp EXCLUDE_FROM_ALL)

add_library(asio_standalone INTERFACE)
add_library(Asio::Asio ALIAS asio_standalone)

target_include_directories(asio_standalone
    SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/asio-1.38.2/include"
)

target_compile_definitions(asio_standalone
    INTERFACE
        ASIO_STANDALONE
)

target_link_libraries(asio_standalone
    INTERFACE
        Threads::Threads
)

if(WIN32)
    target_link_libraries(asio_standalone
        INTERFACE
            ws2_32
            mswsock
    )
    target_compile_definitions(asio_standalone
        INTERFACE
            _WIN32_WINNT=0x0A00
    )
endif()

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.c
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    include/*.hpp
    include/*.h
)

target_sources(${PROJECT_NAME} 
    PRIVATE
        ${SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Asio::Asio
            yaml-cpp::yaml-cpp
)

# Export all symbols from the executable so dynamically loaded modules can access them
# Required for loading dynamic libraries on Windows runtime
if(WIN32)
    target_link_options(${PROJECT_NAME}
        PRIVATE
            -Wl,--export-all-symbols
    )
endif()

# Add support for dynamic library loading
# On Linux/Unix (but not MinGW/MSYS2 on Windows), link against libdl for dynamic library loading
if(UNIX AND NOT APPLE AND NOT WIN32 AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            dl
    )
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_DL_LIBS}
    )
endif()

# Fix for MinGW with GNU compiler linking stdc++exp so std::println works correctly
if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            stdc++exp
    )
endif()

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SOURCES} ${HEADERS}
)



install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${INSTALL_DIR}/bin
    ARCHIVE DESTINATION ${INSTALL_DIR}/lib
    LIBRARY DESTINATION ${INSTALL_DIR}/lib
    FILE_SET HEADERS DESTINATION ${INSTALL_DIR}/include
)

install(
    DIRECTORY config config_joiner
    DESTINATION ${INSTALL_DIR}
)
