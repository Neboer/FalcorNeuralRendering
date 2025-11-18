#include "RenderingServer/SafeMutex.h"
#include <thread>

SafeMutex::SafeMutex() noexcept
    : locked_(false)
{
}

void SafeMutex::lock() noexcept {
    // 自旋直到成功将 locked_ 从 false -> true
    while (locked_.exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
}

void SafeMutex::unlock() noexcept {
    // 与 std::mutex 不同：多次 unlock 安全
    locked_.store(false, std::memory_order_release);
}
