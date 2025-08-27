#include "ABSynchronizer.h"

void ABSynchronizer::requestB()
{
    std::lock_guard<std::mutex> lk(mtx);
    requestBFlag = true;
}

void ABSynchronizer::enterFromA()
{
    std::unique_lock<std::mutex> lk(mtx);
    if (requestBFlag) {
        // 通知B准备好了
        Bready = true;
        cv.notify_all();
        // 等待B执行完成
        cv.wait(lk, [this] { return Bdone; });
        // 重置状态
        Bdone = false;
        Bready = false;
        requestBFlag = false;
    }
}

void ABSynchronizer::BExecute(const std::function<void()>& func)
{
    {
        std::unique_lock<std::mutex> lk(mtx);
        requestBFlag = true;
        cv.notify_all();
        // 等待 A 允许进入
        cv.wait(lk, [this] { return Bready; });
    }
    // 执行 B 的逻辑
    func();
    {
        std::lock_guard<std::mutex> lk(mtx);
        Bdone = true;
        cv.notify_all();
    }
}
