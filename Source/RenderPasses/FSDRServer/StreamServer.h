#pragma once
#include "SimpleSocketServer/SimpleSocketServer.h"
#include <nlohmann/json.hpp>
#include <cstdint>
#include <string>

class StreamServer
{
public:
    explicit StreamServer(int port);
    ~StreamServer();

    // 禁止拷贝
    StreamServer(const StreamServer&) = delete;
    StreamServer& operator=(const StreamServer&) = delete;

    // 允许移动
    StreamServer(StreamServer&&) = default;
    StreamServer& operator=(StreamServer&&) = default;

    // 等待客户端连接（阻塞）
    void waitForClient();

    // 发送 JSON（阻塞）
    void sendJson(const nlohmann::json& j);

    // 接收 JSON（阻塞）
    nlohmann::json receiveJson();

    // 关闭
    void close();

private:
    SimpleSocketServer* socketServer = nullptr;
    SimpleSocket* clientSocket = nullptr;
    int port_;
};
