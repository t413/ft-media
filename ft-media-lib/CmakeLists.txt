get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJ_NAME})

file(GLOB SOURCES "*.cpp" )
file(GLOB HEADERS "*.h" )

list(APPEND HEADERS "addons/CLI11.hpp")

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMT_LIBRARY avformat)
find_library(SWSCALE_LIBRARY swscale)
find_library(AVUTIL_LIBRARY avutil)

find_path(FT_INCLUDE_DIR flaschen-taschen.h HINTS "${FT_LOCATION}/api/include")
find_library(FT_LIBRARY ftclient HINTS "${FT_LOCATION}/api/lib")

find_path(AO_INCLUDE_DIR ao/ao.h)
find_library(AO_LIBRARY ao)


add_library(${PROJ_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJ_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories(${PROJ_NAME} PRIVATE ${AVCODEC_INCLUDE_DIR} ${FT_INCLUDE_DIR} ${AO_INCLUDE_DIR})
target_link_libraries(${PROJ_NAME} PRIVATE
        ${AVCODEC_LIBRARY}
        ${FT_LIBRARY}
        ${AVFORMT_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${AO_LIBRARY}
)
