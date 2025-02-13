#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "Session.h"
#include "SocketUtils.h"

IocpCore::IocpCore(ServerType serverType, NetAddress address, int sessionCount)
	:_serverType(serverType), _netAddress(address), _sessionCount(sessionCount)
{
	SocketUtils::Init(_serverType);
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

IocpCore::~IocpCore()
{
	/*for (AcceptEvent* acceptEvent : _acceptEvents)
		delete(acceptEvent);

	if (_iocpHandle != NULL)
		CloseHandle(_iocpHandle);

	SocketUtils::Clear();
	SocketUtils::Close(_listenSocket);*/
}

void IocpCore::Clear()
{
	for (AcceptEvent* acceptEvent : _acceptEvents)
		delete(acceptEvent);
	for (std::shared_ptr<Session> session : _sessions)
	{
		ReleaseSession(session);
	}

	if (_iocpHandle != NULL)
	{
		CloseHandle(_iocpHandle);
		_iocpHandle = NULL;
	}

	SocketUtils::Clear();
	SocketUtils::Close(_listenSocket);
}

void IocpCore::AddSession(std::shared_ptr<Session> session)
{
	std::lock_guard<std::mutex> lock(_lock);
	_sessionCount++;
	_sessions.insert(session);
}

void IocpCore::ReleaseSession(std::shared_ptr<Session> session)
{
	std::lock_guard<std::mutex> lock(_lock);
	//ASSERT(_sessions.erase(session) != 0); // TODO error
	_sessions.erase(session);
	_sessionCount--;
}

void IocpCore::HandleError(std::string errorMsg)
{
	int errorCode = WSAGetLastError();

	/*CString msg;
	msg.Format(_T("Failed, Code: %d"), errorCode);
	Utils::AlertOK(msg, MB_ICONERROR);*/
}

void IocpCore::SetSocketOption()
{
	int reuseAddr = 1;
	setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuseAddr), sizeof(reuseAddr));

	if (GetServerType() == ServerType::UDP) return;
	LINGER lingerOpt;
	lingerOpt.l_onoff = 0;
	lingerOpt.l_linger = 0;
	setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&lingerOpt), sizeof(lingerOpt));
}

bool IocpCore::StartServerTCP()
{
	_listenSocket = SocketUtils::CreateSocketTCP();
	if (_listenSocket == INVALID_SOCKET)
	{
		int errorCode = WSAGetLastError();
		/*CString msg;
		msg.Format(_T("Failed to start server: %d"), errorCode);
		Utils::AlertOK(msg, MB_ICONERROR);*/
		return false;
	}

	CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, /*key*/0, 0);
	SetSocketOption();
	if (SocketUtils::Bind(_listenSocket, GetNetAddress()) == false) return false;
	if (SocketUtils::Listen(_listenSocket) == false) return false;

	//Utils::AlertOK(_T("Server Set Done"), MB_ICONINFORMATION);
	return true;
}

bool IocpCore::StartServerUDP()
{
	_listenSocket = SocketUtils::CreateSocketUDP();
	if (_listenSocket == INVALID_SOCKET)
	{
		int errorCode = WSAGetLastError();
		cout << "Socket creation failed with error code: " << errorCode << '\n';
		/*CString msg;
		msg.Format(_T("Failed to start UDP server: %d"), errorCode);
		Utils::AlertOK(msg, MB_ICONERROR);*/
		return false;
	}

	CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, /*key*/0, 0);
	SetSocketOption();
	if (SocketUtils::Bind(_listenSocket, GetNetAddress()) == false)
	{
		int errorCode = WSAGetLastError();
		cout << "Binding failed with error code: " << errorCode << '\n';
		return false;
	}
	cout << "Success Listen socket BIND" << '\n';

	return true;
}


std::shared_ptr<Session> IocpCore::CreateSession()
{
	std::shared_ptr<Session> session = std::make_shared<Session>(GetServerType());

	if (GetServerType() == ServerType::TCP) CreateIoCompletionPort(session->GetHandle(), _iocpHandle, 0, 0);

	return session;
}

void IocpCore::StartAcceptTCP()
{
	AcceptEvent* acceptEvent = new AcceptEvent;
	_acceptEvents.push_back(acceptEvent);

	RegisterAcceptTCP(acceptEvent);
}

void IocpCore::RegisterAcceptTCP(AcceptEvent* acceptEvent)
{
	std::shared_ptr<Session> session = CreateSession();
	acceptEvent->Init();
	acceptEvent->sessionRef = session;
	DWORD bytesReceived = 0;

	if (false == SocketUtils::AcceptEx(_listenSocket, session->GetClientSocket(), session->_recvBuffer.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int errorCode = WSAGetLastError();

		if (errorCode != WSA_IO_PENDING)
		{
			// Error -> 다시 Accept 시도
			RegisterAcceptTCP(acceptEvent);
		}
	}
}

void IocpCore::ProcessAcceptTCP(AcceptEvent* acceptEvent)
{
	std::shared_ptr<Session> session = acceptEvent->sessionRef;
	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetClientSocket(), _listenSocket))
	{
		RegisterAcceptTCP(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == getpeername(session->GetClientSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAcceptTCP(acceptEvent);
		return;
	}
	session->SetNetAddress(NetAddress(sockAddress));
	AddSession(session);
	session->ProcessConnectTCP();

	RegisterAcceptTCP(acceptEvent);
}

void IocpCore::StartAcceptUDP()
{
	AcceptEvent* acceptEvent = new AcceptEvent;
	_acceptEvents.push_back(acceptEvent);

	RegisterAcceptUDP(acceptEvent);
}

void IocpCore::RegisterAcceptUDP(AcceptEvent* acceptEvent)
{
	std::shared_ptr<Session> session = CreateSession();
	acceptEvent->Init();
	acceptEvent->sessionRef = session;

	AddSession(session);
	session->ProcessConnectUDP();
}

bool IocpCore::GQCS(UINT timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		cout << "GQCS Event received" << '\n';
		Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int errorCode = WSAGetLastError();

		switch (errorCode)
		{
		case WAIT_TIMEOUT:
			return false;

		default:
			cout << "GetQueuedCompletionStatus failed with error code: " << errorCode << '\n';
			Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}
}

void IocpCore::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
	switch (iocpEvent->_eventType)
	{
	case EventType::Accept:
		cout << "Dispatch -> " << "Accept" << '\n';
		if (GetServerType() == ServerType::TCP) ProcessAcceptTCP(static_cast<AcceptEvent*>(iocpEvent));
		//else if (GetServerType() == ServerType::UDP) ProcessAcceptTCP(static_cast<AcceptEvent*>(iocpEvent));
		break;

	case EventType::Disconnect:
	{
		cout << "Dispatch -> " << "Disconnect" << '\n';
		ReleaseSession(iocpEvent->sessionRef);
		iocpEvent->sessionRef->ProcessDisconnect();
		break;
	}

	case EventType::Recv:
		cout << "Dispatch -> " << "Recv" << '\n';
		if (GetServerType() == ServerType::TCP) iocpEvent->sessionRef->ProcessRecvTCP(numOfBytes);
		else if (GetServerType() == ServerType::UDP) iocpEvent->sessionRef->ProcessRecvUDP(numOfBytes);
		break;

	case EventType::Send:
		cout << "Dispatch -> " << "Send" << '\n';
		if (GetServerType() == ServerType::TCP) iocpEvent->sessionRef->ProcessSendTCP(numOfBytes, static_cast<SendEvent*>(iocpEvent)->sendBuffers);
		else if (GetServerType() == ServerType::UDP) iocpEvent->sessionRef->ProcessSendUDP(numOfBytes, static_cast<SendEvent*>(iocpEvent)->sendBuffers);
		break;

	default:
		break;
	}
}

void IocpCore::Broadcast(std::shared_ptr<SendBuffer> sendBuffer)
{
	std::lock_guard<std::mutex> lock(_lock);
	for (const auto& session : _sessions)
	{
		if (GetServerType() == ServerType::TCP) session->SendTCP(sendBuffer);
		else if (GetServerType() == ServerType::UDP) session->SendUDP(sendBuffer);

	}
}
