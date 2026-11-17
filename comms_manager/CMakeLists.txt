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
    src*.c
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

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Asio::Asio
            yaml-cpp::yaml-cpp
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SOURCES} ${HEADERS}
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    FILE_SET HEADERS DESTINATION include
)