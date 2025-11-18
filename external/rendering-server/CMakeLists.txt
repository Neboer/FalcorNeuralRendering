cmake_minimum_required(VERSION 3.10)
project(RenderingServer LANGUAGES CXX)

# 创建静态库（改为OBJECT库以便更好的继承属性）
add_library(RenderingServer STATIC
    RenderingServer.cpp
    SafeMutex.cpp
    ServiceRegistry.cpp
    RenderingServerOnRender.cpp
)

# 设置包含目录（使用INTERFACE属性以便父项目继承）
target_include_directories(RenderingServer
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

find_package(httplib REQUIRED)

target_link_libraries(RenderingServer PUBLIC httplib::httplib)

