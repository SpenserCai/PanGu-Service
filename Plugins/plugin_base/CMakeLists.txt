cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

project(plugin_base VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(jsoncpp CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(./src SrcFiles)

add_library(plugin_base 
    ${SrcFiles}
)

set(C_DEF_LIST 
    "WIN32_LEAN_AND_MEAN;"
    "_WINSOCK_DEPRECATED_NO_WARNINGS;"
    "_PLUGIN_BASE_LIB_;"
)

target_compile_definitions(plugin_base PRIVATE
    ${C_DEF_LIST}
)

target_link_libraries(plugin_base 
    jsoncpp_lib
    jsoncpp_object
    JsonCpp::JsonCpp
)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
