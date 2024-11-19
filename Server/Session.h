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

	bool IsConnected() { return _connected; }

public:
	bool Disconnect(const WCHAR* cause);
	void Send(shared_ptr<SendBuffer> sendBuffer);

private:
	void RegisterRecv();
	void ProcessRecv(int32 numOfBytes);
	int32 OnRecv(BYTE* buffer, int32 len);
	int32 OnSend(int32 len, vector<shared_ptr<SendBuffer>> sendVec);

	void RegisterSend();
	void ProcessSend(int32 numOfBytes, vector<shared_ptr<SendBuffer>> sendVec);


private:
	atomic<bool> _connected = false;
	SOCKET _clientSocket = INVALID_SOCKET;
	SOCKADDR_IN _clientAddr;

	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;

	RecvBuffer _recvBuffer;
	queue<shared_ptr<SendBuffer>> _sendQueue;
	atomic<bool> _sendRegistered = false;

	mutex _lock;
};

