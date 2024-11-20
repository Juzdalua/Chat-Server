#pragma once
#include "Session.h"

class IocpCore
{
public:
	IocpCore(NetAddress address, int32 sessionCount = 1);
	~IocpCore();

	NetAddress GetNetAddress() { return _netAddress; }

	int32 GetMaxSessionCount() { return _sessionCount; }
	void AddSession(shared_ptr<Session> session);
	void ReleaseSession(shared_ptr<Session> session);

	void HandleError(string errorMsg);

	void SetSocketOption();
	bool StartServer();

	shared_ptr<Session> CreateSession();
	void StartAccept();
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

	bool Dispatch(uint32 timeoutMs = INFINITE);
	

private:
	NetAddress _netAddress = {};

	int32 _sessionCount = 1;
	SOCKET _listenSocket = INVALID_SOCKET;
	SOCKADDR_IN _serverAddr = { 0 };
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
	
	vector<AcceptEvent*> _acceptEvents;
	set<shared_ptr<Session>> _sessions;

	mutex _lock;
};

