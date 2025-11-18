#include "RenderingServer/ServiceRegistry.h"

ServiceRegistry& ServiceRegistry::instance() {
    static ServiceRegistry inst;
    return inst;
}

void ServiceRegistry::registerServer(std::shared_ptr<RenderingServer> server) {
    mServer = std::move(server);
}

std::shared_ptr<RenderingServer> ServiceRegistry::getServer() const {
    return mServer;
}
