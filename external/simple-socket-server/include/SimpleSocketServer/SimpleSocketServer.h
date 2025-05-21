#ifndef SIMPLE_SOCKET_SERVER_H
#define SIMPLE_SOCKET_SERVER_H

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#endif

#include <stdexcept>
#include <string>

class SimpleSocket {
public:
    SimpleSocket(SOCKET sock = INVALID_SOCKET);
    ~SimpleSocket();

    // 禁用拷贝构造和赋值
    SimpleSocket(const SimpleSocket&) = delete;
    SimpleSocket& operator=(const SimpleSocket&) = delete;

    // 添加移动语义
    SimpleSocket(SimpleSocket&& other) noexcept;
    SimpleSocket& operator=(SimpleSocket&& other) noexcept;

    size_t read(char* buffer, size_t size);
    bool read_exact(char* buffer, size_t total);

    size_t write(const char* buffer, size_t size);
    void close();

private:
    SOCKET sock_;
};

class SimpleSocketServer {
public:
    SimpleSocketServer(int port);
    ~SimpleSocketServer();

    SimpleSocket* accept();
    void close();

private:
    void initialize();
    void cleanup();

    SOCKET server_socket_;
    int port_;
    bool initialized_;
#ifdef _WIN32
    WSADATA wsa_data_;
#endif
};

#endif // SIMPLE_SOCKET_SERVER_H
