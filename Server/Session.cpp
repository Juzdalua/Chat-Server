#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include <MSWSock.h>
#include "PacketQueue.h"
//#include "ClientPacketHandler.h"

Session::Session(ServerType serverType)
	:_recvBuffer(BUFFER_SIZE)
{
	if (serverType == ServerType::TCP) _clientSocket = SocketUtils::CreateSocketTCP();
	else if (serverType == ServerType::UDP) _clientSocket = SocketUtils::CreateSocketUDP();
	if (_clientSocket == INVALID_SOCKET)
	{
		cout << "Session Init Error" << '\n';
	}
}

Session::~Session()
{
	//Utils::AlertOK(_T("Session Close"), MB_ICONINFORMATION);

	SocketUtils::Close(_clientSocket);
}

/*----------------
	Disconnect
----------------*/
bool Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false) return false;

	_disconnectEvent.Init();
	_disconnectEvent.sessionRef = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_clientSocket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.sessionRef = nullptr;
			return false;
		}
	}

	return true;
}

void Session::ProcessConnectTCP()
{
	_connectEvent.sessionRef = nullptr;
	_connected.store(true);

	OnConnected();

	RegisterRecvTCP();
}

void Session::ProcessConnectUDP()
{
	_connectEvent.sessionRef = nullptr;
	_connected.store(true);

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(1998); // 원하는 포트 설정

	if (bind(_clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		cout << "UDP Bind Failed. Error Code: " << errorCode << '\n';
		return;
	}

	OnConnected();

	RegisterRecvUDP();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.sessionRef = nullptr;
	OnDisconnected();
}

/*----------------
	Recv
----------------*/
void Session::RegisterRecvTCP()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.sessionRef = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();
	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (WSARecv(_clientSocket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.sessionRef = nullptr;
		}
	}
}

void Session::ProcessRecvTCP(int numOfBytes)
{
	_recvEvent.sessionRef = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int dataSize = _recvBuffer.DataSize();
	int processLen = OnRecvTCP(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	// 수신 재등록
	RegisterRecvTCP();
}

void Session::RegisterRecvUDP()
{
	if (IsConnected() == false)
	{
		cout << "Session is not connect" << '\n';
		return;
	}

	_recvEvent.Init();
	_recvEvent.sessionRef = shared_from_this();

	DWORD flags = 0;
	DWORD recvBytes = 0;
	sockaddr_in senderAddr;
	int senderAddrSize = sizeof(senderAddr);

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	int result = WSARecvFrom(_clientSocket, &wsaBuf, 1, &recvBytes, &flags,
		reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrSize,
		&_recvEvent, nullptr);

	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // IOCP에 정상 등록된 경우가 아니라면
		{
			cout << "RegisterRecvUDP Error: " << errorCode << '\n';
			HandleError(errorCode);
			_recvEvent.sessionRef = nullptr;
		}
	}
}

void Session::ProcessRecvUDP(int numOfBytes)
{
	_recvEvent.sessionRef = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int dataSize = _recvBuffer.DataSize();
	int processLen = OnRecvUDP(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	// 수신 재등록
	RegisterRecvUDP();
}

int Session::OnRecvTCP(unsigned char* buffer, int len)
{
	int processLen = 0;
	while (true)
	{
		int dataSize = len - processLen;

		// 최소 헤더 크기만큼은 있어야한다. => 헤더 파싱
		if (dataSize < sizeof(PacketHeader))
			break;

		// *((PacketHeader*)&buffer[0])
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// 헤더에 기록된 패킷 크기를 파싱할 수 있어야 한다.
		if (dataSize < header.size)
			break;

		// 패킷 조립 성공
		//pktQueue->Push({ buffer, len, nullptr });

		std::shared_ptr<PacketData> packetData = std::make_shared<PacketData>();
		packetData->buffer = buffer + processLen;  // 데이터 버퍼
		packetData->len = header.size;
		pktQueue->Push(packetData);

		processLen += header.size;
	}

	return processLen;
}

int Session::OnRecvUDP(unsigned char* buffer, int len)
{
	int processLen = 0;
	while (true)
	{
		int dataSize = len - processLen;

		// 최소 헤더 크기만큼은 있어야한다. => 헤더 파싱
		if (dataSize < sizeof(PacketHeader))
			break;

		// 헤더 파싱
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// 패킷 크기 검증
		if (dataSize < header.size)
			break;

		// 유효한 패킷을 큐에 넣기
		std::shared_ptr<PacketData> packetData = std::make_shared<PacketData>();
		packetData->buffer = buffer + processLen;  // 데이터 버퍼
		packetData->len = header.size;
		pktQueue->Push(packetData);

		processLen += header.size;
	}

	return processLen;
}


int Session::OnSend(int len, std::vector<std::shared_ptr<SendBuffer>> sendVec)
{
	/*if (sendVec.size() > 0)
	{
		std::shared_ptr<Session> session = shared_from_this();
		PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(sendVec.back()->Buffer());

		CString msg;
		msg.Format(_T("Send len = %d"), len);
		Utils::AlertOK(msg, MB_ICONINFORMATION);
	}*/

	return len;
}

void Session::OnConnected()
{
	//Utils::AlertOK(_T("OnConnected"), MB_ICONINFORMATION);
}

void Session::OnDisconnected()
{
	//Utils::AlertOK(_T("OnDisconnected"), MB_ICONINFORMATION);
}

/*----------------
	Send
----------------*/
void Session::SendTCP(std::shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false) return;

	bool registerSend = false;
	{
		std::lock_guard<std::mutex> lock(_lock);
		_sendQueue.push(sendBuffer);
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend) RegisterSendTCP();
}

void Session::SendUDP(std::shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false) return;

	bool registerSend = false;
	{
		std::lock_guard<std::mutex> lock(_lock);
		_sendQueue.push(sendBuffer);
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend) RegisterSendUDP();
}

void Session::RegisterSendTCP()
{
	if (IsConnected() == false) return;

	_sendEvent.Init();
	_sendEvent.sessionRef = shared_from_this();
	{
		std::lock_guard<std::mutex> lock(_lock);
		int writeSize = 0;

		while (_sendQueue.empty() == false)
		{
			std::shared_ptr<SendBuffer> sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();
			// TODO 예외체크 -> 센드버퍼보다 데이터 크기가 클 경우

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	std::vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (std::shared_ptr<SendBuffer> sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == WSASend(_clientSocket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.sessionRef = nullptr;
			_sendEvent.sendBuffers.clear();
			_sendRegistered.store(false);

		}
	}
}

void Session::ProcessSendTCP(int numOfBytes, std::vector<std::shared_ptr<SendBuffer>> sendVec)
{
	_sendEvent.sessionRef = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}
	OnSend(numOfBytes, sendVec);
	{
		std::lock_guard<std::mutex> lock(_lock);
		if (_sendQueue.empty())
		{
			_sendRegistered.store(false);
			return;
		}
	}
	RegisterSendTCP();
}

void Session::RegisterSendUDP()
{
	if (IsConnected() == false) return;

	_sendEvent.Init();
	_sendEvent.sessionRef = shared_from_this();

	sockaddr_in destAddr; // UDP에서 보낼 대상 주소 설정
	// destAddr 설정 (예: 서버 주소 설정)

	std::vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (std::shared_ptr<SendBuffer> sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == sendto(_clientSocket, reinterpret_cast<char*>(wsaBufs.data()), wsaBufs.size(), 0,
		reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr)))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.sessionRef = nullptr;
			_sendEvent.sendBuffers.clear();
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessSendUDP(int numOfBytes, std::vector<std::shared_ptr<SendBuffer>> sendVec)
{
	_sendEvent.sessionRef = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	OnSend(numOfBytes, sendVec);
	{
		std::lock_guard<std::mutex> lock(_lock);
		if (_sendQueue.empty())
		{
			_sendRegistered.store(false);
			return;
		}
	}

	// UDP에서는 전송 실패 처리를 좀 더 유연하게 할 수 있지만, 바로 재시도하는 등의 처리는 선택 사항입니다.
	//RegisterSendUDP();
}


void Session::HandleError(int errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
	case WSAESHUTDOWN:
	case ERROR_NETNAME_DELETED:
	{
		/*CString msg;
		msg.Format(_T("Close: %d"), errorCode);
		Utils::AlertOK(msg, MB_ICONERROR);*/
		Disconnect(L"HandleError");
		break;
	}

	default:
		// TODO: to log therad
		/*CString msg;
		msg.Format(_T("Handle Error: %d"), errorCode);
		Utils::AlertOK(msg, MB_ICONERROR);*/
		break;
	}
}
