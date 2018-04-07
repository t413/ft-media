cmake_minimum_required (VERSION 3.1)
project(ft-media)

set(CMAKE_CXX_STANDARD 11)
set(CXX_EXTENSIONS NO)
set(CXX_STANDARD_REQUIRED YES)

set(FT_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../flaschen-taschen" CACHE PATH "Location of flaschen-taschen project")
message(STATUS "FT_LOCATION -> ${FT_LOCATION}")

add_subdirectory(ft-media-lib)

add_executable(ft-media-server main.cpp)
target_link_libraries(ft-media-server ft-media-lib)
