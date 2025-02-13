#pragma once
#include "Session.h"

#ifndef SERVER_TYPE_H
#define SERVER_TYPE_H

enum class ServerType
{
	TCP,
	UDP
};

#endif // SERVER_TYPE_H


class IocpCore
{
public:
	IocpCore(ServerType serverType, NetAddress address, int sessionCount = 1);
	~IocpCore();

	void Clear();

	ServerType GetServerType() { return _serverType; }
	NetAddress GetNetAddress() { return _netAddress; }

	int GetMaxSessionCount() { return _sessionCount; }
	void AddSession(std::shared_ptr<Session> session);
	void ReleaseSession(std::shared_ptr<Session> session);
	int GetCurrentSessionCount() { return _sessions.size(); }

	void HandleError(std::string errorMsg);

	void SetSocketOption();
	bool StartServerTCP();
	bool StartServerUDP();

	std::shared_ptr<Session> CreateSession();
	void StartAcceptTCP();
	void RegisterAcceptTCP(AcceptEvent* acceptEvent);
	void ProcessAcceptTCP(AcceptEvent* acceptEvent);

	void StartAcceptUDP();
	void RegisterAcceptUDP(AcceptEvent* acceptEvent);

	bool GQCS(UINT timeoutMs = INFINITE);
	void Dispatch(IocpEvent* iocpEvent, int numOfBytes);

	void Broadcast(std::shared_ptr<SendBuffer> sendBuffer);

private:
	NetAddress _netAddress = {};
	ServerType _serverType;

	int _sessionCount = 1;
	SOCKET _listenSocket = INVALID_SOCKET;
	SOCKADDR_IN _serverAddr = { 0 };
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;

	std::vector<AcceptEvent*> _acceptEvents;
	std::set<std::shared_ptr<Session>> _sessions;

	std::mutex _lock;
};

