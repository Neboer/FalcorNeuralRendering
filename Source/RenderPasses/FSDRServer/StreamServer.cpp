#include "StreamServer.h"
#include <stdexcept>
#include <vector>
#include <cstring>     // memcpy

StreamServer::StreamServer(int port) : port_(port)
{
    socketServer = new SimpleSocketServer(port_);
}

StreamServer::~StreamServer()
{
    close();
    delete socketServer;
}

void StreamServer::waitForClient()
{
    if (clientSocket)
    {
        throw std::runtime_error("Client already connected");
    }
    clientSocket = socketServer->accept();
    if (!clientSocket)
    {
        throw std::runtime_error("Failed to accept client connection");
    }
}

void StreamServer::sendJson(const nlohmann::json& j)
{
    if (!clientSocket)
    {
        throw std::runtime_error("No client connected");
    }

    std::string data = j.dump(); // JSON -> string
    uint32_t len = static_cast<uint32_t>(data.size());

    // 转为网络字节序
    uint32_t net_len = htonl(len);

    // 先发送长度
    clientSocket->write(reinterpret_cast<const char*>(&net_len), sizeof(net_len));
    // 再发送数据
    clientSocket->write(data.data(), data.size());
}

nlohmann::json StreamServer::receiveJson()
{
    if (!clientSocket)
    {
        throw std::runtime_error("No client connected");
    }

    uint32_t net_len = 0;
    if (!clientSocket->read_exact(reinterpret_cast<char*>(&net_len), sizeof(net_len)))
    {
        throw std::runtime_error("Failed to read message length");
    }

    uint32_t len = ntohl(net_len);
    if (len == 0)
    {
        throw std::runtime_error("Invalid message length 0");
    }

    std::vector<char> buffer(len);
    if (!clientSocket->read_exact(buffer.data(), len))
    {
        throw std::runtime_error("Failed to read message body");
    }

    std::string json_str(buffer.begin(), buffer.end());
    return nlohmann::json::parse(json_str);
}

void StreamServer::close()
{
    if (clientSocket)
    {
        clientSocket->close();
        delete clientSocket;
        clientSocket = nullptr;
    }
}
