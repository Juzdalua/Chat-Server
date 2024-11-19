#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "Session.h"
#include "SocketUtils.h"

int PORT = 7777;

IocpCore::~IocpCore()
{
	SocketUtils::Close(_listenSocket);

	if (_iocpHandle != NULL)
		CloseHandle(_iocpHandle);

	WSACleanup();
}

void IocpCore::HandleError(string errorMsg)
{
	int errorCode = WSAGetLastError();
	cout << errorMsg << ", Code: " << errorCode << '\n';
}

bool IocpCore::Start()
{
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0)
	{
		HandleError("WSAStartup");
		return false;
	}
	return true;
}

void IocpCore::CreateSocket()
{
	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == INVALID_SOCKET) HandleError("TCP Set");
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

void IocpCore::ServerSet()
{
	memset(&_serverAddr, 0, sizeof(_serverAddr));
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_serverAddr.sin_port = htons(PORT);
}

bool IocpCore::Bind()
{
	if (::bind(_listenSocket, (SOCKADDR*)&_serverAddr, sizeof(_serverAddr)) == SOCKET_ERROR)
	{
		HandleError("Bind");
		return false;
	}
	return true;
}

bool IocpCore::Listen()
{
	if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		HandleError("Listen");
		return false;
	}
	return true;
}

void IocpCore::StartServer()
{
	if (!Start()) return;
	CreateSocket();
	SetSocketOption();
	ServerSet();
	if (!Bind()) return;
	if (!Listen()) return;
	cout << "Server Set Done" << '\n';
}

void IocpCore::SetIOCP()
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

bool IocpCore::Accept(shared_ptr<Session> session)
{
	int addrLen = sizeof(session->_clientAddr);
	session->_clientSocket = accept(_listenSocket, (SOCKADDR*)&(session->_clientAddr), &addrLen);
	if (session->_clientSocket == INVALID_SOCKET)
	{
		HandleError("Accept");
		return false;
	}
	return true;
}

void IocpCore::Register(shared_ptr<Session> session)
{
	CreateIoCompletionPort((HANDLE)(session->_clientSocket), _iocpHandle, 0, 0);
	RegisterRecv(session);
}

void IocpCore::RegisterRecv(shared_ptr<Session> session)
{
	WSABUF wsaBuf;
	DWORD numOfBytes = 0;
	DWORD flags = 0;

	//WSARecv(session->_clientSocket, &wsaBuf, 1, &numOfBytes, &flags, )
}

void IocpCore::StartAccept(shared_ptr<Session> session)
{
	SetIOCP();
	cout << "Wait Accept..." << '\n';
	while (true)
	{
		if (!Accept(session))
		{
			continue;
		}
		break;
	}
	Register(session);
	cout << "Accept Done" << '\n';
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	BOOL ret = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs);
	if (ret == FALSE || numOfBytes == 0)
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
	else
	{
		// Dispatch
	}
}
