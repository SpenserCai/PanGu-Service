cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

project(pangu-service VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()


find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(PolyHook_2 CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_package(c-ares CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(CPP_BASE64_INCLUDE_DIRS "cpp-base64/base64.cpp")


set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs/x64)
link_directories(${LIBS_DIR})


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/json)

include_directories(${PROJECT_SOURCE_DIR}/include/ServiceModule)
# include_directories(${PROJECT_SOURCE_DIR}/include/ServiceModule/DbPlugin)
include_directories(${PROJECT_SOURCE_DIR}/protos)

include_directories(${PROJECT_SOURCE_DIR}/../Plugins/plugin_base/include)


aux_source_directory(./src SrcFiles)
aux_source_directory(./src/lib_json SrcFiles)
aux_source_directory(./src/ServiceModule SrcFiles)
aux_source_directory(./src/ServiceModule/ServiceUtils SrcFiles)
aux_source_directory(./protos Protos)
aux_source_directory(${CPP_BASE64_INCLUDE_DIRS}/cpp-base64 SrcFiles)
add_executable(pangu-service ${SrcFiles}
    ${Protos}
)

#add_library(pangu-service ${SrcFiles}
#    ${Protos}
#)

set(C_DEF_LIST 
    "WIN32_LEAN_AND_MEAN;"
    "_WINSOCK_DEPRECATED_NO_WARNINGS;"
    "_CRT_SECURE_NO_DEPRECATE;"
    "_PLUGIN_BASE_LIB_;"
)

# 使用PHLIB必须这么设置！！！
target_compile_definitions(pangu-service PRIVATE
    ${C_DEF_LIST}
)


target_include_directories(pangu-service PRIVATE ${CPP_BASE64_INCLUDE_DIRS})

# 注意编译phlib时 C/C++->代码生成->运行库 设置位多线程Dll(/MD)
target_link_libraries(pangu-service
    ${PROJECT_SOURCE_DIR}/../Plugins/plugin_base/build/Release/plugin_base.lib
    re2::re2 
    c-ares::cares
    gRPC::gpr 
    gRPC::upb 
    gRPC::grpc 
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotoc 
    protobuf::libprotobuf 
    protobuf::libprotobuf-lite
    cpr::cpr
    unofficial::sqlite3::sqlite3
    PolyHook_2::PolyHook_2
    OpenSSL::SSL
    OpenSSL::Crypto
    ntdll
    iphlpapi.lib
    ws2_32.lib
    wintrust.lib
    crypt32.lib
    # Userenv.lib
    # WtsApi32.lib
)

#获取程序输出路径
set(PLUGIN_DIR ${CMAKE_BINARY_DIR}/Release/Plugins)
message(STATUS "Plugin dir: ${PLUGIN_DIR}")
make_directory(${PLUGIN_DIR})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

