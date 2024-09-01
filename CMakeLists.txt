cmake_minimum_required(VERSION 3.22)
project(client_server_sockets)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/files)

set(CMAKE_C_COMPILER gcc)

get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE) #get absolute path to project root directory
set(BIN_DIR "${PROJECT_ROOT_DIR}/bin") #set bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}) #set bin directory as output directory

set(SOURCES_C src/clients.c src/middle.c cJSON/cJSON.c)
set(HEADERS_C inc/clients.h inc/middle.h inc/common.h cJSON/cJSON.h)

set(SOURCES_S src/server.c src/middle.c src/server_utils.c cJSON/cJSON.c)
set(HEADERS_S inc/server.h inc/middle.h inc/server_utils.h inc/common.h cJSON/cJSON.h)

add_executable(clients ${SOURCES_C} ${HEADERS_C})
add_executable(server ${SOURCES_S} ${HEADERS_S})

target_compile_options(clients PRIVATE -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 -g)
target_compile_options(server PRIVATE -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 -g)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

target_link_libraries(clients PRIVATE ${ZLIB_LIBRARIES})
target_link_libraries(server PRIVATE ${ZLIB_LIBRARIES})
