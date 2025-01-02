#pragma once
#include "Session.h"

class IocpCore
{
public:
	IocpCore(NetAddress address, int sessionCount = 1);
	~IocpCore();

	void Clear();

	NetAddress GetNetAddress() { return _netAddress; }

	int GetMaxSessionCount() { return _sessionCount; }
	void AddSession(std::shared_ptr<Session> session);
	void ReleaseSession(std::shared_ptr<Session> session);
	int GetCurrentSessionCount() { return _sessions.size(); }

	void HandleError(std::string errorMsg);

	void SetSocketOption();
	bool StartServer();

	std::shared_ptr<Session> CreateSession();
	void StartAccept();
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

	bool GQCS(UINT timeoutMs = INFINITE);
	void Dispatch(IocpEvent* iocpEvent, int numOfBytes);

	void Broadcast(std::shared_ptr<SendBuffer> sendBuffer);

private:
	NetAddress _netAddress = {};

	int _sessionCount = 1;
	SOCKET _listenSocket = INVALID_SOCKET;
	SOCKADDR_IN _serverAddr = { 0 };
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;

	std::vector<AcceptEvent*> _acceptEvents;
	std::set<std::shared_ptr<Session>> _sessions;

	std::mutex _lock;
};

