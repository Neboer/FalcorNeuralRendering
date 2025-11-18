#pragma once
#include "RenderingServer/RenderingServer.h"
#include <memory>

class ServiceRegistry {
public:
    static ServiceRegistry& instance();

    void registerServer(std::shared_ptr<RenderingServer> server);
    std::shared_ptr<RenderingServer> getServer() const;

private:
    ServiceRegistry() = default;
    std::shared_ptr<RenderingServer> mServer;
};
