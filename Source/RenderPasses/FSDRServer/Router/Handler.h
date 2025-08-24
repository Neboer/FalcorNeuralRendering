#pragma once
#include <nlohmann/json.hpp>

class Router;

class Handler {
public:
    virtual ~Handler() = default;

    // 在 router.addHandler 时调用
    virtual void routerRegister(Router* router) = 0;

protected:
    Router* router_ = nullptr;
};

