#pragma once
#include <winsock2.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include "SendBuffer.h"

struct sockaddr_in_compare {
    bool operator()(const sockaddr_in& lhs, const sockaddr_in& rhs) const {
        return lhs.sin_addr.s_addr < rhs.sin_addr.s_addr ||
            (lhs.sin_addr.s_addr == rhs.sin_addr.s_addr && lhs.sin_port < rhs.sin_port);
    }
};

class UDPServer
{
public:
    UDPServer(string ip, u_short port);
    ~UDPServer();

    void Init();
    void Bind();
    void Recv();
    void AddClient(const sockaddr_in& clientAddr);
    void RemoveClient(const sockaddr_in& clientAddr);
    void Broadcast(shared_ptr<SendBuffer> sendBuffer);

private:
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;
    char buffer[0x10000];
    int clientAddrSize;
    string _ip;
    u_short _port;
    std::set<sockaddr_in, sockaddr_in_compare> clientAddresses;
};

