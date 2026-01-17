cmake_minimum_required(VERSION 3.10)
project(Tokenizer CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(re2 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

set(SOURCES
    pretokenizer.cpp
    dataset.cpp
    tokenizer.cpp
    main.cpp
)

add_executable(tokenizer ${SOURCES})

target_include_directories(tokenizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tokenizer
    arrow_shared
    parquet_shared
    re2::re2
)