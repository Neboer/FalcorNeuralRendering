#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

class ABSynchronizer {
public:
    // B 请求插队
    void requestB();

    // A 每次循环时调用，如果 B 请求了，则 A 会等待 B 完成
    void enterFromA();

    // B 执行逻辑的封装
    void BExecute(const std::function<void()>& func);

private:
    std::mutex mtx;
    std::condition_variable cv;

    bool requestBFlag = false;
    bool Bready = false;
    bool Bdone = false;
};

