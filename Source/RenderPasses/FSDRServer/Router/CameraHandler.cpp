#include "Handler.h"
#include "Router.h"
#include <iostream>

class CameraHandler : public Handler {
public:
    void routerRegister(Router* router) override {
        this->router_ = router;
        router->on("setCameraPosition", [this](const nlohmann::json& msg) {
            this->setCameraPosition(msg);
        });
    }

    void setCameraPosition(const nlohmann::json& msg) {
        if (msg.contains("x") && msg.contains("y") && msg.contains("z")) {
            float x = msg["x"], y = msg["y"], z = msg["z"];
            std::cout << "Camera position set to (" << x << ", " << y << ", " << z << ")\n";
        }
    }
};
