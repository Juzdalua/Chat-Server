#pragma once
#include "RecvBuffer.h"

struct PacketHeader
{
	uint16 size; // 패킷 size
	uint16 id; // 프로토콜 ID (ex 1=로그인, 2=이동요청)
};

class Session
{
	friend class IocpCore;

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
	SOCKET _clientSocket = INVALID_SOCKET;
	SOCKADDR_IN _clientAddr;

	RecvBuffer _recvBuffer;
};

