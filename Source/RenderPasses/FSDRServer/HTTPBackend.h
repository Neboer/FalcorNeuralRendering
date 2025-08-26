#pragma once
#include <nlohmann/json.hpp>
#include <cstdint>
#include <string>
#include "Utils/Threading.h"
#include "httplib.h"
#include "RenderGraph/RenderPassHelpers.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include <Core/API/RenderContext.h>
#include <optional>

struct FalcorContext {
    const Falcor::ChannelList* channels;
    Falcor::RenderContext* pRenderContext;
    std::optional<std::reference_wrapper<const Falcor::RenderData>> renderData;
    Falcor::IScene* pScene;
};

class HTTPBackend
{
public:
    explicit HTTPBackend(std::string host, int port, FalcorContext context);
    ~HTTPBackend();

    // 禁止拷贝
    HTTPBackend(const HTTPBackend&) = delete;
    HTTPBackend& operator=(const HTTPBackend&) = delete;

    // 允许移动
    HTTPBackend(HTTPBackend&&) = default;
    HTTPBackend& operator=(HTTPBackend&&) = default;


    void SetRenderingContext(Falcor::RenderContext* pRenderContext, const Falcor::RenderData& renderData);

    // 关闭
    void close();

private:
    FalcorContext context;
    httplib::Server server;
    std::optional<Falcor::Threading::Task> serverTask;
};
