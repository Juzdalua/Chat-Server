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

/*----------------
	Disconnect
----------------*/
bool Session::Disconnect(const WCHAR* cause)
{
	_disconnectEvent.Init();
	_disconnectEvent.sessionRef = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_clientSocket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.sessionRef = nullptr;
			return false;
		}
	}

	return true;
}

/*----------------
	Recv
----------------*/
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
	ClientPacketHandler::HandlePacket(buffer, len, shared_from_this());
	
	// Echo
	cout << "OnRecv Data = " << buffer << endl;
	/*shared_ptr<SendBuffer> sendBuffer = shared_ptr<SendBuffer>(new SendBuffer(4096));
	sendBuffer->CopyData(buffer, len);
	Send(sendBuffer);*/
	
	return len;
}

int32 Session::OnSend(int32 len, vector<shared_ptr<SendBuffer>> sendVec)
{
	if (sendVec.size() > 0)
	{
		shared_ptr<Session> session = shared_from_this();
		PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(sendVec.back()->Buffer());

		cout << "Send len = " << len << '\n';
		//HandlePacketStartLog("SEND", LogColor::BLUE, recvHeader, session);
	}

	return len;
}

/*----------------
	Send
----------------*/
void Session::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false) return;

	bool registerSend = false;
	{
		lock_guard<mutex> lock(_lock);
		_sendQueue.push(sendBuffer);
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend) RegisterSend();
}

void Session::RegisterSend()
{
	if (IsConnected() == false) return;

	_sendEvent.Init();
	_sendEvent.sessionRef = shared_from_this();
	{
		lock_guard<mutex> lock(_lock);
		int32 writeSize = 0;

		while (_sendQueue.empty() == false)
		{
			shared_ptr<SendBuffer> sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();
			// TODO 예외체크 -> 센드버퍼보다 데이터 크기가 클 경우

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (shared_ptr<SendBuffer> sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == WSASend(_clientSocket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			cout << "Send Error" << '\n';
			_sendEvent.sessionRef = nullptr;
			_sendEvent.sendBuffers.clear(); 
			_sendRegistered.store(false);

		}
	}
}

void Session::ProcessSend(int32 numOfBytes, vector<shared_ptr<SendBuffer>> sendVec)
{
	_sendEvent.sessionRef = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numOfBytes == 0)
	{
		//Disconnect(L"Send 0");
		return;
	}
	OnSend(numOfBytes, sendVec);
	{
		lock_guard<mutex> lock(_lock);
		if (_sendQueue.empty()) 
		{
			_sendRegistered.store(false);
			return;
		}
	}
	RegisterSend();
}
