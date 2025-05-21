#include "SimpleSocketServer/SimpleSocketServer.h"
#include <cstring>
#include <sstream>

SimpleSocket::SimpleSocket(SOCKET sock) : sock_(sock) {}

SimpleSocket::~SimpleSocket() { close(); }

SimpleSocket::SimpleSocket(SimpleSocket&& other) noexcept : sock_(other.sock_)
{
    other.sock_ = INVALID_SOCKET; // 防止原对象关闭套接字
}
SimpleSocket& SimpleSocket::operator=(SimpleSocket&& other) noexcept
{
    if (this != &other)
    {
        close();
        sock_ = other.sock_;
        other.sock_ = INVALID_SOCKET;
    }
    return *this;
}

size_t SimpleSocket::read(char* buffer, size_t size) {
    if (sock_ == INVALID_SOCKET) return 0;
    
    int received = recv(sock_, buffer, static_cast<int>(size), 0);
    if (received <= 0) {
        close();
        return 0;
    }
    return static_cast<size_t>(received);
}

bool SimpleSocket::read_exact(char* buffer, size_t total)
{
    if (sock_ == INVALID_SOCKET)
    {
        throw std::runtime_error("read_exact: Socket already closed");
    }

    if (total == 0)
    {
        throw std::invalid_argument("Total size must be greater than 0.");
    }

    size_t offset = 0;
    while (offset < total)
    {
        int received = recv(sock_, buffer + offset, static_cast<int>(total - offset), 0);
        if (received < 0)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
            std::ostringstream oss;
            oss << "recv() failed with WSA error code " << err;
            throw std::runtime_error(oss.str());
#else
            if (errno == EINTR)
                continue; // interrupted by signal, retry

            std::ostringstream oss;
            oss << "recv() failed: " << strerror(errno) << " (errno " << errno << ")";
            throw std::runtime_error(oss.str());
#endif
        }

        if (received == 0)
        {
            // Connection closed by peer
            return false;
        }

        offset += static_cast<size_t>(received);
    }

    return true;
}


size_t SimpleSocket::write(const char* buffer, size_t size) {
    if (sock_ == INVALID_SOCKET) return 0;
    
    int sent = send(sock_, buffer, static_cast<int>(size), 0);
    if (sent <= 0) {
        close();
        return 0;
    }
    return static_cast<size_t>(sent);
}

void SimpleSocket::close() {
    if (sock_ != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock_);
#else
        ::close(sock_);
#endif
        sock_ = INVALID_SOCKET;
    }
}

SimpleSocketServer::SimpleSocketServer(int port) 
    : port_(port), server_socket_(INVALID_SOCKET), initialized_(false) {
    initialize();
}

SimpleSocketServer::~SimpleSocketServer() {
    close();
    cleanup();
}

void SimpleSocketServer::initialize() {
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif

    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed");
    }

    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, 
              (const char*)&opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_socket_, SOMAXCONN) == SOCKET_ERROR) {
        throw std::runtime_error("Listen failed");
    }

    initialized_ = true;
}

SimpleSocket* SimpleSocketServer::accept() {
    sockaddr_in client_addr{};
#ifdef _WIN32
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    SOCKET client_socket = ::accept(server_socket_, 
                                   (sockaddr*)&client_addr, 
                                   &addr_len);
    if (client_socket == INVALID_SOCKET) {
        throw std::runtime_error("Accept failed");
    }
    return new SimpleSocket(client_socket);
}

void SimpleSocketServer::close() {
    if (server_socket_ != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(server_socket_);
#else
        ::close(server_socket_);
#endif
        server_socket_ = INVALID_SOCKET;
    }
}

void SimpleSocketServer::cleanup() {
#ifdef _WIN32
    if (initialized_) {
        WSACleanup();
        initialized_ = false;
    }
#endif
}
