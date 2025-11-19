#pragma once
#include "httplib.h"
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include "RenderingServer/SafeMutex.h"
#include <thread>
#include <optional>
#include <map>

// 用于处理渲染请求的服务器类，包裹httplib中的服务器对象，提供一些方便的方法。
// 整个服务器只有一个这样的实例，采用静态方法进行访问。
// 服务器与客户端通信方法使用BJData格式。
class RenderingServer
{
private:
    httplib::Server server;
    std::thread serverThread;

    static nlohmann::json MakeResponse(bool success, std::string errorData = "");
    static void SendBJDataResponse(httplib::Response& res, int code, const nlohmann::json& jsonData);
    static void SendJSONDataResponse(httplib::Response& res, int code, const nlohmann::json& jsonData);

    void BindSetCropWindow();
    void BindServerHello();
    void BindTestSafeMutex();
    void BindRenderingHandler();
    void BindGetRenderingResultData();

    // 接下来所有的private都是实现渲染相关的辅助功能，同步原语等等。用于 BindRenderingHandler 与 FSDRServer 配合使用
    // 实现目标——把Falcor变成一个HTTP协议的渲染器。
    SafeMutex renderingMutex; // 用于限制同时只有一个渲染HTTP请求在处理的锁。
    // 以下两个锁用于在FSDRServer与server之间同步一次渲染的请求与渲染的结果。
    SafeMutex renderingRequestSyncMutex;  // 与NotifyRenderingRequest、WaitForRenderingRequest配合使用的锁。
    SafeMutex renderingCompleteSyncMutex; // 与NotifyRenderingComplete、WaitForRenderingComplete配合使用的锁，同上。

public:
    // set_crop_window 用于设置裁剪窗口的API操作的变量
    nlohmann::json cropWindow;
    // set_param_and_render 用于设置参数并渲染的API操作的变量
    SafeMutex testMutex;                                // 仅仅是用来测试SafeMutex用的锁，并无实际用途，创建时锁住。

    // 渲染请求与渲染结果同步相关的方法，用于在FSDRServer与server之间同步一次渲染的请求与渲染的结果。
    void WaitForRenderingRequest();
    void NotifyRenderingRequest();
    void WaitForRenderingComplete();
    void NotifyRenderingComplete();

    // 渲染请求的缓存数据
    nlohmann::json sceneParamCache = {}; // 请求者要求的场景参数的缓存，结构化定义 SetSceneParamReq
    nlohmann::json renderingResultInfoCache = {}; // 缓存渲染结果数据，注意这里并没有实际的二进制图像。
    std::map<std::string, std::vector<uint8_t>> renderingResultDataCache; // 存储实际的二进制图像数据，key为图像名称。

    RenderingServer(std::string host, int port);

    // 禁止拷贝
    RenderingServer(const RenderingServer&) = delete;
    RenderingServer& operator=(const RenderingServer&) = delete;

    // 允许移动
    RenderingServer(RenderingServer&&) = default;
    RenderingServer& operator=(RenderingServer&&) = default;
};
