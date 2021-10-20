cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME
        "server"
)

project(${PROJECT_NAME})

find_package (Threads REQUIRED)

set(SOURCES
    main.cpp
    server.cpp
    logger.cpp
)

set(HEADERS
    server.h
    logger.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
