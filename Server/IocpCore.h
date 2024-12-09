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

	bool GQCS(uint32 timeoutMs = INFINITE);
	void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes);
	
	bool CanBroadcast() { if (_sessions.size() > 0) return true; return false; }
	void Broadcast(std::shared_ptr<SendBuffer> sendBuffer);

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

