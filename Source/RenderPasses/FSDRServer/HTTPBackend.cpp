#include "HTTPBackend.h"
#include <stdexcept>
#include <cstring> // memcpy

HTTPBackend::HTTPBackend(std::string host, int port, FalcorContext context) : context(context)
{
    server.Get("/hi", [](const httplib::Request&, httplib::Response& res) { res.set_content("Hello World!", "text/plain"); });
    server.Get(
        "/channel",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->context.pABSync->BExecute(
                [this, &req, &res]()
                {
                    nlohmann::json response_json;

                    if (this->context.renderData)
                    {
                        for (auto& channel : *this->context.channels)
                        {
                            nlohmann::json channel_json;
                            auto& channelName = channel.name;
                            auto& rd = this->context.renderData->get();
                            auto tex = rd.getTexture(channelName);
                            if (tex)
                            {
                                channel_json["width"] = tex->getWidth();
                                channel_json["height"] = tex->getHeight();
                                channel_json["format"] = getFormatChannelCount(tex->getFormat());
                                channel_json["bytesPerPixel"] = getFormatBytesPerBlock(tex->getFormat());
                            }
                            else
                            {
                                res.status = 500;
                                res.set_content(fmt::format("Channel {} texture not found", channelName), "text/plain");
                                return;
                            }
                            response_json[channelName] = channel_json;
                        }
                        res.set_content(response_json.dump(), "application/json");
                        res.status = 200;
                        return;
                    }
                    else
                    {
                        res.status = 500;
                        res.set_content("RenderData not set", "text/plain");
                        return;
                    }
                }
            );
        }
    );
    server.Get(
        "/channel/:channel_name",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->context.pABSync->BExecute(
                [this, &req, &res]()
                {
                    std::string channelName = req.path_params.at("channel_name");
                    if (this->context.renderData && this->context.pRenderContext)
                    {
                        auto& rd = this->context.renderData->get();
                        auto tex = rd.getTexture(channelName);
                        if (tex)
                        {
                            // Read texture data
                            size_t dataSize = tex->getWidth() * tex->getHeight() * getFormatBytesPerBlock(tex->getFormat());
                            std::vector<uint8_t> data(dataSize);
                            uint32_t subresource = tex->getSubresourceIndex(0, 0);
                            this->context.pRenderContext->readTextureSubresource(tex.get(), subresource);
                            // Send as binary
                            res.set_content(reinterpret_cast<const char*>(data.data()), dataSize, "application/octet-stream");
                            res.status = 200;
                            return;
                        }
                        else
                        {
                            res.status = 404;
                            res.set_content("Channel not found", "text/plain");
                            return;
                        }
                    }
                    else
                    {
                        res.status = 500;
                        res.set_content("RenderData or RenderContext not set", "text/plain");
                        return;
                    }
                }
            );
        }
    );
    server.Post(
        "/scene/set_camera",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            this->context.pABSync->BExecute(
                [this, &req, &res]()
                {
                    if (this->context.pScene)
                    {
                        try
                        {
                            nlohmann::json j = nlohmann::json::parse(req.body);

                            auto cameraControl = Falcor::float3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());

                            Falcor::logInfo(fmt::format(
                                "Received camera position: x = {}, y = {}, z = {}", cameraControl.x, cameraControl.y, cameraControl.z
                            ));
                            this->context.pScene->getCamera()->setPosition(cameraControl);

                            res.status = 400;
                            res.set_content("Invalid camera parameters", "text/plain");
                            return;
                        }
                        catch (const std::exception& e)
                        {
                            res.status = 400;
                            res.set_content(std::string("Failed to parse JSON: ") + e.what(), "text/plain");
                            return;
                        }
                    }
                    else
                    {
                        res.status = 500;
                        res.set_content("Scene not set", "text/plain");
                        return;
                    }
                }
            );
        }
    );
    server.Get(
        "/exit",
        [this](const httplib::Request&, httplib::Response& res)
        {
            res.set_content("Shutting down server", "text/plain");
            res.status = 200;
            exit(0);
        }
    );

    serverTask = Falcor::Threading::Task(Falcor::Threading::dispatchTask([this, host, port]() { server.listen(host, port); }));

    if (!server.is_valid())
    {
        throw std::runtime_error("Failed to start HTTP server");
    }
    else
    {
        Falcor::logInfo("HTTP server started at " + host + ":" + std::to_string(port));
    }
}

void HTTPBackend::SetRenderingContext(Falcor::RenderContext* pRenderContext, const Falcor::RenderData& renderData)
{
    context.pRenderContext = pRenderContext;
    context.renderData = renderData;
}

HTTPBackend::~HTTPBackend()
{
    server.stop();
}
