#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "ClientPacketHandler.h"

Session::Session()
	:_recvBuffer(BUFFER_SIZE)
{
	_clientSocket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_clientSocket);
}

void Session::RegisterRecv()
{
	WSABUF wsaBuf = { 0 };
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	_recvEvent.Init();
	_recvEvent.sessionRef = shared_from_this();

	if (WSARecv(_clientSocket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			cout << "WSARecv Error" << '\n';
		}
	}
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.sessionRef = nullptr;
	if (numOfBytes == 0)
	{
		// TODO
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		//Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	// 수신 재등록
	RegisterRecv();
}

int32 Session::OnRecv(BYTE* buffer, int32 len)
{
	cout << "OnRecv Data = " << buffer << endl;
	
	ClientPacketHandler::HandlePacket(buffer, len, shared_from_this());

	/*shared_ptr<SendBuffer> sendBuffer = shared_ptr<SendBuffer>(new SendBuffer(4096));
	sendBuffer->CopyData(buffer, len);*/
	return len;
}
