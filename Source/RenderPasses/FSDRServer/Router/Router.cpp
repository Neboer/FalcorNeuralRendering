#include "Router.h"
#include "Handler.h"
#include <iostream>

Router::Router(int port) {
    server_ = std::make_unique<StreamServer>(port);
}

Router::~Router() {
}

void Router::addHandler(std::shared_ptr<Handler> handler) {
    handler->routerRegister(this);
}

void Router::on(const std::string& type, std::function<void(const nlohmann::json&)> handler) {
    eventHandlers_[type].push_back(handler);
}

void Router::waitAndHandleOneRequest() {
    try {
        nlohmann::json request = server_->recvJson(); // 阻塞接收 JSON
        if (!request.contains("type")) {
            std::cerr << "Invalid request: no type field\n";
            return;
        }

        std::string type = request["type"];
        auto it = eventHandlers_.find(type);
        if (it != eventHandlers_.end()) {
            for (auto& handler : it->second) {
                handler(request); // 调用所有注册的处理函数
            }
        } else {
            std::cerr << "No handler registered for type: " << type << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling request: " << e.what() << std::endl;
    }
}
