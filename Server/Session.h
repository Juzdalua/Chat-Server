#pragma once
#include "RecvBuffer.h"
#include "IocpEvent.h"

struct PacketHeader
{
	uint16 size; // 패킷 size
	uint16 id; // 프로토콜 ID (ex 1=로그인, 2=이동요청)
};

class Session : public enable_shared_from_this<Session>
{
	friend class IocpCore;
	friend class IocpEvent;

	enum
	{
		BUFFER_SIZE = 0x10000 // 64KB
	};

public:
	Session();
	~Session();

public:
	SOCKET GetClientSocket() { return _clientSocket; }
	SOCKADDR_IN GetClientAddr() { return _clientAddr; }

private:
	void RegisterRecv();
	void ProcessRecv(int32 numOfBytes);
	int32 OnRecv(BYTE* buffer, int32 len);

private:
	SOCKET _clientSocket = INVALID_SOCKET;
	SOCKADDR_IN _clientAddr;

	RecvBuffer _recvBuffer;

	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};

