#include "RenderingServer/RenderingServer.h"

// 渲染服务器的具体实现，实际的渲染请求在这里处理。这里负责总的渲染请求，传入场景参数，获得所有渲染结果。
void RenderingServer::BindRenderingHandler()
{
    // 完整获取所有GBuffer与渲染结果的接口
    this->server.Post(
        "/render/full",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->renderingMutex.lock(); // 保证同一时间只有一个渲染请求在处理。
            this->sceneParamCache = nlohmann::json::parse(req.body);
            this->NotifyRenderingRequest();   // 通知FSDRServer有渲染请求来了。
            this->WaitForRenderingComplete(); // 等待FSDRServer渲染完成。
            this->SendJSONDataResponse(res, 200, this->renderingResultInfoCache);
            this->renderingMutex.unlock();
        }
    );
}

// 获取渲染结果的二进制数据接口 /renderdata/<data_key>
void RenderingServer::BindGetRenderingResultData()
{
    this->server.Get(
        R"(/renderdata/(\w+))",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            std::string dataKey = req.matches[1];
            auto it = this->renderingResultDataCache.find(dataKey);
            if (it != this->renderingResultDataCache.end())
            {
                const std::vector<uint8_t>& binaryData = it->second;
                res.status = 200;
                res.set_content(reinterpret_cast<const char*>(binaryData.data()), binaryData.size(), "application/octet-stream");
            }
            else
            {
                nlohmann::json errorJson = MakeResponse(false, "Data key not found: " + dataKey);
                SendBJDataResponse(res, 404, errorJson);
            }
        }
    );
}

void RenderingServer::WaitForRenderingRequest()
{
    this->renderingRequestSyncMutex.lock();
}

void RenderingServer::NotifyRenderingRequest()
{
    this->renderingRequestSyncMutex.unlock();
}

void RenderingServer::WaitForRenderingComplete()
{
    this->renderingCompleteSyncMutex.lock();
}

void RenderingServer::NotifyRenderingComplete()
{
    this->renderingCompleteSyncMutex.unlock();
}
