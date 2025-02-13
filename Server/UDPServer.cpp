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
	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup failed" << '\n';
		exit(1);
	}

	// ���� ���� ����
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "Socket creation failed" << '\n';
		exit(1);
	}

	// ���� �ּ� ����
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, _ip.c_str(), &serverAddr.sin_addr);
}

void UDPServer::Bind()
{
	// ���ε�
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
			RemoveClient(clientAddr); // ���� ������ Ŭ���̾�Ʈ ����
		}
		return;
	}

	buffer[recvLen] = '\0';

	if (std::find_if(clientAddresses.begin(), clientAddresses.end(),
		[this](const sockaddr_in& client) { return CompareSockaddrIn(client, clientAddr); })
		== clientAddresses.end()) {
		AddClient(clientAddr); // Ŭ���̾�Ʈ �ּ� �߰�
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
	// Ŭ���̾�Ʈ �ּ� ����Ʈ(clientAddresses)�� ��ȸ
	for (const auto& clientAddr : clientAddresses)
	{
		// WSABUF ����ü�� �غ��Ͽ� sendBuffer �����͸� ����
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());

		// clientAddr�� �����͸� ����
		int sentBytes = sendto(serverSocket, wsaBuf.buf, wsaBuf.len, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));

		// ���� ���� �� ���� �ڵ� ���
		if (sentBytes == SOCKET_ERROR) {
			std::cout << "sendto failed, ErrorCode: " << WSAGetLastError() << '\n';
		}
	}
}

