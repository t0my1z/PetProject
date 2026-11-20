cmake_minimum_required(VERSION 3.23)

# This CMakeLists.txt manages building modules as shared libraries
# Each module directory should contain its own CMakeLists.txt that uses this as a reference

# Note: Modules are built relative to the main comms_manager project
# They should link against the comms_manager installation to get headers and libraries

# Example module CMakeLists.txt (put in module subdirectory):
#
# cmake_minimum_required(VERSION 3.23)
# project(chat_creator VERSION 1.0 LANGUAGES CXX)
# 
# set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 
# # Find the installed comms_manager package or use direct paths
# find_package(asio REQUIRED)
# 
# add_library(chat_creator SHARED)
# 
# target_sources(chat_creator
#     PRIVATE
#         src/ChatCreator.cpp
# )
# 
# target_include_directories(chat_creator
#     PRIVATE
#         ${COMMS_MANAGER_INCLUDE_DIR}
# )
# 
# target_link_libraries(chat_creator
#     PRIVATE
#         asio
# )
# 
# install(
#     TARGETS chat_creator
#     LIBRARY DESTINATION ../install/modules/chat_creator
#     RUNTIME DESTINATION ../install/modules/chat_creator
# )
