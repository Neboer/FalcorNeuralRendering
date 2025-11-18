#include "RenderingServer/RenderingServer.h"

// 渲染服务器的具体实现，实际的渲染请求在这里处理。这里负责总的渲染请求，传入场景参数，获得所有渲染结果。
void RenderingServer::BindRenderingHandler()
{
    this->server.Post("/render/full", [this](const httplib::Request& req, httplib::Response& res) {

    });
}
