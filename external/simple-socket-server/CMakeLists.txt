cmake_minimum_required(VERSION 3.10)
project(SimpleSocketServer LANGUAGES CXX)

# 创建静态库（改为OBJECT库以便更好的继承属性）
add_library(SimpleSocketServer STATIC
    SimpleSocketServer.cpp
)

# 设置包含目录（使用INTERFACE属性以便父项目继承）
target_include_directories(SimpleSocketServer
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# 平台特定设置
if(WIN32)
    target_link_libraries(SimpleSocketServer PRIVATE ws2_32)
    target_compile_definitions(SimpleSocketServer PRIVATE _WIN32_WINNT=0x0601)
else()
    target_link_libraries(SimpleSocketServer PRIVATE pthread)
endif()
