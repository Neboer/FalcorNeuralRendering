#include "RenderingServer/RenderingServer.h"

RenderingServer::RenderingServer(std::string host, int port) :/* server(),*/ cropWindow{0, 0, 0, 0}
{
    renderMutex.lock(); // 一开始锁住，等有渲染请求时再解锁。
    server.Get("/hi", [](const httplib::Request&, httplib::Response& res) { res.set_content("Hello World!", "text/plain"); });

    // server listen on thread
    server.bind_to_port(host, port);
    serverThread = std::thread([this]() { this->server.listen_after_bind(); });
}
