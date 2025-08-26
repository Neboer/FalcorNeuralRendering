#include "HTTPBackend.h"
#include <stdexcept>
#include <cstring> // memcpy

HTTPBackend::HTTPBackend(std::string host, int port, FalcorContext context) : context(context)
{
    server.Get("/hi", [](const httplib::Request&, httplib::Response& res) { res.set_content("Hello World!", "text/plain"); });
    serverTask = Falcor::Threading::Task(Falcor::Threading::dispatchTask([this, host, port]() { server.listen(host, port); }));
}

void HTTPBackend::SetRenderingContext(Falcor::RenderContext* pRenderContext, const Falcor::RenderData& renderData)
{
    context.pRenderContext = pRenderContext;
    context.renderData = renderData;
}

HTTPBackend::~HTTPBackend()
{
    close();
}

void HTTPBackend::close() {}
