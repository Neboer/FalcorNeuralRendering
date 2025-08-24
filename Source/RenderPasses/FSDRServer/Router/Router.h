#pragma once

#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderPassHelpers.h"
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../StreamServer.h"

// 前向声明
class Handler;

// Router 类
class Router {
public:
    struct RenderingData {
        Falcor::ref<Falcor::IScene> Scene = nullptr;       // ref<IScene> 的指针占位符
        Falcor::RenderContext* RenderContext = nullptr; // RenderContext* 的占位符
        Falcor::ChannelList* Channels = nullptr;           // Falcor::ChannelList* 的占位符
    };

    explicit Router(int port);
    ~Router();

    // RenderingData 可以由外部直接修改
    RenderingData renderingData;

    // 添加处理器（handler）
    void addHandler(std::shared_ptr<Handler> handler);

    // 注册事件
    void on(const std::string& type, std::function<void(const nlohmann::json&)> handler);

    // 等待并处理一个请求
    void waitAndHandleOneRequest();

private:
    std::unique_ptr<StreamServer> server_;
    std::map<std::string, std::vector<std::function<void(const nlohmann::json&)>>> eventHandlers_;
};
