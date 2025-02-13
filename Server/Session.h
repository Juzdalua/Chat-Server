#pragma once
#include "RecvBuffer.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include <memory>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>

#ifndef SERVER_TYPE_H
#define SERVER_TYPE_H

enum class ServerType
{
	TCP,
	UDP
};

#endif // SERVER_TYPE_H


struct PacketHeader
{
	UINT size;
	UINT id;
};

enum class ServiceType : UINT
{
	Server,
	Client,
};


class Session : public std::enable_shared_from_this<Session>
{
	friend class IocpCore;
	friend class IocpEvent;

	enum
	{
		BUFFER_SIZE = 0x10000 // 64KB
	};

public:
	Session(ServerType serverType);
	~Session();

public:
	SOCKET GetClientSocket() { return _clientSocket; }
	SOCKADDR_IN GetClientAddr() { return _clientAddr; }

	void SetServiceType(ServiceType type) { _serviceType = type; }

	void SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress GetAddress() { return _netAddress; }

	bool IsConnected() { return _connected; }

public:
	bool Disconnect(const WCHAR* cause);
	void SendTCP(std::shared_ptr<SendBuffer> sendBuffer);
	void SendUDP(std::shared_ptr<SendBuffer> sendBuffer);

private:
	HANDLE GetHandle() { return reinterpret_cast<HANDLE>(_clientSocket); }

	void ProcessConnectTCP();
	void ProcessConnectUDP();
	void ProcessDisconnect();

	void RegisterRecvTCP();
	void ProcessRecvTCP(int numOfBytes);

	void RegisterRecvUDP();
	void ProcessRecvUDP(int numOfBytes);

	void RegisterSendTCP();
	void ProcessSendTCP(int numOfBytes, std::vector<std::shared_ptr<SendBuffer>> sendVec);

	void RegisterSendUDP();
	void ProcessSendUDP(int numOfBytes, std::vector<std::shared_ptr<SendBuffer>> sendVec);

	void HandleError(int errorCode);

private:
	void OnConnected();
	void OnDisconnected();
	int OnRecvTCP(unsigned char* buffer, int len);
	int OnRecvUDP(unsigned char* buffer, int len);
	int OnSend(int len, std::vector<std::shared_ptr<SendBuffer>> sendVec);

private:
	NetAddress _netAddress = {};
	ServiceType _serviceType = ServiceType::Client;

	std::atomic<bool> _connected = false;
	SOCKET _clientSocket = INVALID_SOCKET;
	SOCKADDR_IN _clientAddr;

	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;

	RecvBuffer _recvBuffer;
	std::queue<std::shared_ptr<SendBuffer>> _sendQueue;
	std::atomic<bool> _sendRegistered = false;

	std::mutex _lock;
};

