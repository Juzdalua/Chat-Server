#pragma once
#include "Session.h"

class IocpCore
{
public:
	IocpCore() = default;
	~IocpCore();

	void HandleError(string errorMsg);

	bool Start();
	void CreateSocket();
	void SetSocketOption();
	void ServerSet();
	bool Bind();
	bool Listen();
	void StartServer();

	void SetIOCP();
	bool Accept(shared_ptr<Session> session);
	void Register(shared_ptr<Session> session);
	void RegisterRecv(shared_ptr<Session> session);
	void StartAccept(shared_ptr<Session> session);

	bool Dispatch(uint32 timeoutMs = INFINITE);


private:
	WSAData _wsaData;
	SOCKET _listenSocket = INVALID_SOCKET;
	SOCKADDR_IN _serverAddr;
	HANDLE _iocpHandle;
};

