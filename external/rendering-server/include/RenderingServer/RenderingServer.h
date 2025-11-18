#pragma once
#include "httplib.h"
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include "RenderingServer/SafeMutex.h"
#include <thread>
#include <optional>


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

public:
    // set_crop_window 用于设置裁剪窗口的API操作的变量
    nlohmann::json cropWindow;
    // set_param_and_render 用于设置参数并渲染的API操作的变量
    std::function<void(nlohmann::json)> sendRenderData; // 如果渲染结果准备好了，通过此函数发送。
    SafeMutex renderMutex; // 创建后锁住，如果有渲染请求则由RenderingServer释放。FSDRServer必须锁上此锁才能动作。
    nlohmann::json sceneParams; // 请求者要求的场景参数的缓存，结构化定义 SetSceneParamReq

    RenderingServer(std::string host, int port);
    
    // 禁止拷贝
    RenderingServer(const RenderingServer&) = delete;
    RenderingServer& operator=(const RenderingServer&) = delete;

    // 允许移动
    RenderingServer(RenderingServer&&) = default;
    RenderingServer& operator=(RenderingServer&&) = default;
};
