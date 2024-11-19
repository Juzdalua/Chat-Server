#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

Session::Session()
	:_recvBuffer(BUFFER_SIZE)
{
	_clientSocket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_clientSocket);
}
