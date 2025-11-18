#pragma once
#include <atomic>

class SafeMutex {
public:
    SafeMutex() noexcept;
    ~SafeMutex() noexcept = default;

    // 禁止拷贝与移动
    SafeMutex(const SafeMutex&) = delete;
    SafeMutex& operator=(const SafeMutex&) = delete;

    void lock() noexcept;
    void unlock() noexcept;

private:
    std::atomic<bool> locked_;
};
