#include "pch.h"
#include "UDPServer.h"
#include "PacketQueue.h"

bool CompareSockaddrIn(const sockaddr_in& a, const sockaddr_in& b)
{
	return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

UDPServer::UDPServer(string ip, u_short port)
	:_ip(ip), _port(port), clientAddrSize(sizeof(clientAddr))
{
	serverSocket = INVALID_SOCKET;
}

UDPServer::~UDPServer()
{
	if (serverSocket != INVALID_SOCKET)
	{
		closesocket(serverSocket);
	}
	WSACleanup();
}

void UDPServer::Init()
{
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup failed" << '\n';
		exit(1);
	}

	// 서버 소켓 생성
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "Socket creation failed" << '\n';
		exit(1);
	}

	// 서버 주소 설정
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, _ip.c_str(), &serverAddr.sin_addr);
}

void UDPServer::Bind()
{
	// 바인딩
	/*if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Binding failed" << '\n';
		exit(1);
	}*/

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Bind failed, ErrorCode: " << WSAGetLastError() << '\n';
		return;
	}

	std::cout << "Server is listening on port " << _port << std::endl;
}

void UDPServer::Recv()
{
	int recvLen = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&clientAddr, &clientAddrSize);
	if (recvLen == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		std::cout << "recvfrom failed, ErrorCode: " << errorCode << '\n';

		if (errorCode == WSAECONNRESET) {
			RemoveClient(clientAddr); // 연결 끊어진 클라이언트 제거
		}
		return;
	}

	buffer[recvLen] = '\0';

	if (std::find_if(clientAddresses.begin(), clientAddresses.end(),
		[this](const sockaddr_in& client) { return CompareSockaddrIn(client, clientAddr); })
		== clientAddresses.end()) {
		AddClient(clientAddr); // 클라이언트 주소 추가
	}

	auto packetData = std::make_shared<PacketData>();
	packetData->buffer = reinterpret_cast<unsigned char*>(buffer);
	packetData->len = recvLen;
	pktQueue->Push(packetData);
}

void UDPServer::AddClient(const sockaddr_in& clientAddr) {
	clientAddresses.insert(clientAddr);
}

void UDPServer::RemoveClient(const sockaddr_in& clientAddr) {
	clientAddresses.erase(clientAddr);
}

void UDPServer::Broadcast(std::shared_ptr<SendBuffer> sendBuffer)
{
	// 클라이언트 주소 리스트(clientAddresses)를 순회
	for (const auto& clientAddr : clientAddresses)
	{
		// WSABUF 구조체를 준비하여 sendBuffer 데이터를 복사
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());

		// clientAddr에 데이터를 전송
		int sentBytes = sendto(serverSocket, wsaBuf.buf, wsaBuf.len, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));

		// 전송 실패 시 에러 코드 출력
		if (sentBytes == SOCKET_ERROR) {
			std::cout << "sendto failed, ErrorCode: " << WSAGetLastError() << '\n';
		}
	}
}

