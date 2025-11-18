#include "RenderingServer/RenderingServer.h"

// 我们所有的数据交换都使用BJData格式进行编码和解码。
nlohmann::json RenderingServer::MakeResponse(bool success, std::string errorData)
{
    nlohmann::json responseJson;
    responseJson["status"] = success ? "success" : "error";
    if (!success)
    {
        responseJson["message"] = errorData;
    }
    return responseJson;
}

void RenderingServer::SendBJDataResponse(httplib::Response& res, int code, const nlohmann::json& jsonData)
{
    res.status = code;
    std::vector<uint8_t> bjdata = nlohmann::json::to_bjdata(jsonData);
    char* p = reinterpret_cast<char*>(bjdata.data());
    size_t len = bjdata.size();
    res.set_content(p, len, "application/bjdata");
}

void RenderingServer::SendJSONDataResponse(httplib::Response& res, int code, const nlohmann::json& jsonData)
{
    res.status = code;
    std::string jsonString = jsonData.dump();
    res.set_content(jsonString, "application/json");
}

void RenderingServer::BindSetCropWindow()
{
    server.Post(
        "/set_crop_window",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                this->cropWindow = nlohmann::json::parse(req.body);
                SendJSONDataResponse(res, 200, MakeResponse(true));
            }
            catch (const std::exception& e)
            {
                SendJSONDataResponse(res, 400, MakeResponse(false, e.what()));
            }
        }
    );
}

void RenderingServer::BindServerHello()
{
    server.Get(
        "/hello",
        [](const httplib::Request& req, httplib::Response& res)
        {
            nlohmann::json responseJson;
            responseJson["message"] = "Hello from RenderingServer!";
            responseJson["value"] = 42;
            responseJson["version"] = "0.2.1";
            SendJSONDataResponse(res, 200, responseJson);
        }
    );

    server.Get("/stop", [](const httplib::Request& req, httplib::Response& res) { exit(0); });
}

void RenderingServer::BindTestSafeMutex()
{
    server.Get(
        "/safe_mutex/lock",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->renderMutex.lock();
            SendJSONDataResponse(res, 200, MakeResponse(true));
        }
    );
    server.Get(
        "/safe_mutex/unlock",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->renderMutex.unlock();
            SendJSONDataResponse(res, 200, MakeResponse(true));
        }
    );
}

RenderingServer::RenderingServer(std::string host, int port) : cropWindow{{"x", 0}, {"y", 0}, {"width", 100}, {"height", 100}}
{
    renderMutex.lock(); // 一开始锁住，等有渲染请求时再解锁。
    BindServerHello();
    BindSetCropWindow();
    BindTestSafeMutex();

    // server listen on thread
    server.bind_to_port(host, port);
    serverThread = std::thread([this]() { this->server.listen_after_bind(); });
}
