#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "Session.h"
#include "SocketUtils.h"

int PORT = 7777;

IocpCore::IocpCore(NetAddress address, int32 sessionCount)
	:_netAddress(address), _sessionCount(sessionCount)
{
	SocketUtils::Init();
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

IocpCore::~IocpCore()
{
	if (_iocpHandle != NULL)
		CloseHandle(_iocpHandle);

	SocketUtils::Clear();
	SocketUtils::Close(_listenSocket);
}

void IocpCore::AddSession(shared_ptr<Session> session)
{
	lock_guard<mutex> lock(_lock);
	_sessionCount++;
	_sessions.insert(session);
}

void IocpCore::ReleaseSession(shared_ptr<Session> session)
{
	lock_guard<mutex> lock(_lock);
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}

void IocpCore::HandleError(string errorMsg)
{
	int errorCode = WSAGetLastError();
	cout << errorMsg << ", Code: " << errorCode << '\n';
}

void IocpCore::SetSocketOption()
{
	int reuseAddr = 1;
	setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuseAddr), sizeof(reuseAddr));

	LINGER lingerOpt;
	lingerOpt.l_onoff = 0;
	lingerOpt.l_linger = 0;
	setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&lingerOpt), sizeof(lingerOpt));
}

bool IocpCore::StartServer()
{
	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == INVALID_SOCKET)
	{
		int32 errorCode = WSAGetLastError();
		cout << errorCode << endl;
		return false;
	}

	CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, /*key*/0, 0);
	SetSocketOption();
	if (SocketUtils::Bind(_listenSocket, GetNetAddress()) == false) return false;
	if (SocketUtils::Listen(_listenSocket) == false) return false;

	cout << "Server Set Done" << '\n';
	return true;
}

shared_ptr<Session> IocpCore::CreateSession()
{
	shared_ptr<Session> session = make_shared<Session>();
	CreateIoCompletionPort(session->GetHandle(), _iocpHandle, 0, 0);

	return session;
}

void IocpCore::StartAccept()
{
	AcceptEvent* acceptEvent = new AcceptEvent;
	_acceptEvents.push_back(acceptEvent);

	RegisterAccept(acceptEvent);
}

void IocpCore::RegisterAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<Session> session = CreateSession();
	acceptEvent->Init();
	acceptEvent->sessionRef = session;
	DWORD bytesReceived = 0;

	if (false == SocketUtils::AcceptEx(_listenSocket, session->GetClientSocket(), session->_recvBuffer.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errorCode = WSAGetLastError();

		if (errorCode != WSA_IO_PENDING)
		{
			// Error -> 다시 Accept 시도
			RegisterAccept(acceptEvent);
		}
	}
}

void IocpCore::ProcessAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<Session> session = acceptEvent->sessionRef;
	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetClientSocket(), _listenSocket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == getpeername(session->GetClientSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}
	session->SetNetAddress(NetAddress(sockAddress));
	AddSession(session);
	session->ProcessConnect();

	RegisterAccept(acceptEvent);
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if(GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		switch (iocpEvent->_eventType)
		{
		case EventType::Accept:
			ProcessAccept(static_cast<AcceptEvent*>(iocpEvent));
			break;

		case EventType::Disconnect:
			iocpEvent->sessionRef->ProcessDisconnect();
			ReleaseSession(iocpEvent->sessionRef);
			break;

		case EventType::Recv:
			iocpEvent->sessionRef->ProcessRecv(numOfBytes);
			break;

		case EventType::Send:
			iocpEvent->sessionRef->ProcessSend(numOfBytes, static_cast<SendEvent*>(iocpEvent)->sendBuffers);
			break;

		default:
			break;
		}
	}
	else
	{
		int errorCode = WSAGetLastError();
		switch (errorCode)
		{
		case WAIT_TIMEOUT:
			return false;

		case WSA_IO_PENDING:
			return false;

		default:
			// 
			break;
		}
	}
}
