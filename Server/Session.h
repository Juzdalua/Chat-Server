#pragma once
#include "RecvBuffer.h"

struct PacketHeader
{
	uint16 size; // ��Ŷ size
	uint16 id; // �������� ID (ex 1=�α���, 2=�̵���û)
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

