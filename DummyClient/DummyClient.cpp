#define WIN32_LEAN_AND_MEAN   
#include <windows.h>
#include <iostream>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <thread>
#include <json.hpp>

using namespace std;
using json = nlohmann::json;

struct PacketHeader
{
	UINT size; // 패킷 size
	UINT id; // 프로토콜 ID (ex 1=로그인, 2=이동요청)
	UINT seq;
};

enum class ServerType
{
	TCP,
	UDP
};

void RawSend(SOCKET& clientSocket, ServerType& serverType, SOCKADDR_IN& serverAddr)
{
	while (true)
	{
		cout << "ID를 입력하세요: ";
		int id;
		cin >> id;
		
		cout << '\n';

		cout << "데이터를 입력하세요: ";
		string s;
		cin >> s;

		json jsonData = {
			{"data", s},
		};

		std::string jsonString = jsonData.dump(); // JSON -> String
		UINT jsonSize = static_cast<UINT>(jsonString.size());
		int totalPacketSize = jsonSize + sizeof(PacketHeader);

		//UINT id = 5000U;
		UINT id = 4000U;
		PacketHeader sendHeader = { 0 };
		sendHeader.id = id;
		sendHeader.size = sizeof(PacketHeader) + jsonString.size();

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, &sendHeader, sizeof(PacketHeader));
		memcpy(sendBuffer + sizeof(PacketHeader), jsonString.data(), jsonSize);
		if (serverType == ServerType::TCP)
		{
			// TCP일 때는 send() 사용
			int resultCode = send(clientSocket, sendBuffer, totalPacketSize, 0);
			if (resultCode == SOCKET_ERROR)
			{
				int errCode = WSAGetLastError();
				cout << "Send ErrorCode: " << errCode << '\n';
				return;
			}
		}
		else if (serverType == ServerType::UDP)
		{
			// UDP일 때는 sendto() 사용

			int resultCode = sendto(clientSocket, sendBuffer, totalPacketSize, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
			if (resultCode == SOCKET_ERROR)
			{
				int errCode = WSAGetLastError();
				cout << "Send ErrorCode: " << errCode << '\n';
				return;
			}
		}
		cout << "[SEND] ID -> " << sendHeader.id << ", SIZE -> " << sendHeader.size << '\n';
	}
}

void RawRecv(SOCKET& clientSocket, ServerType& serverType)
{
	char recvBuffer[4096]; // 수신 버퍼
	int processLen = 0;    // 처리된 데이터 길이

	while (true)
	{
		int recvLen = 0;

		if (clientSocket == INVALID_SOCKET)
		{
			cout << "INVALID SOCKET" << '\n';
			return;
		}

		if (serverType == ServerType::TCP)
		{
			// TCP에서는 recv() 사용
			recvLen = recv(clientSocket, recvBuffer + processLen, sizeof(recvBuffer) - processLen, 0);
		}
		else if (serverType == ServerType::UDP)
		{
			// UDP에서는 recvfrom() 사용
			SOCKADDR_IN senderAddr;
			memset(&senderAddr, 0, sizeof(senderAddr));  // senderAddr 초기화
			int senderAddrLen = sizeof(senderAddr);
			recvLen = recvfrom(clientSocket, recvBuffer + processLen, sizeof(recvBuffer) - processLen, 0, (SOCKADDR*)&senderAddr, &senderAddrLen);
		}

		cout << "RecvLen: " << recvLen << '\n';

		if (recvLen <= 0)
		{
			int error = WSAGetLastError();
			std::cout << "Receive Error: " << error << '\n';
			return;
		}

		processLen += recvLen;
		int dataSize = processLen;

		while (dataSize > 0)
		{
			// 최소 헤더 크기 확인
			if (dataSize < sizeof(PacketHeader))
				break;

			// 헤더 파싱
			PacketHeader* header = reinterpret_cast<PacketHeader*>(recvBuffer);

			// 패킷 크기 확인
			if (dataSize < header->size)
				break;

			// 데이터 처리
			cout << '\n' << "[RECV] ID -> " << header->id << ", SEQ -> " << header->seq << ", SIZE -> " << header->size << ", LEN => " << recvLen << '\n';

			if (header->size > sizeof(PacketHeader))
			{
				const char* payload = recvBuffer + sizeof(PacketHeader);
				int payloadSize = header->size - sizeof(PacketHeader);

				std::string jsonString(payload, payloadSize);
				std::cout << "JSON Data: " << jsonString << '\n';
			}

			// 처리된 데이터 만큼 버퍼 이동
			int packetSize = header->size;
			memmove(recvBuffer, recvBuffer + packetSize, processLen - packetSize);
			processLen -= packetSize;
			dataSize = processLen;
		}
	}
}

int main()
{
	this_thread::sleep_for(1s);
	// 1. Socket 초기화
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	ServerType serverType = ServerType::TCP;
	//ServerType serverType = ServerType::UDP;

	/*
		af: Address Family (AF_INET = IPv4, AF_INTE6 = IPv6)
		type: TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
		protocol: 0
		return: descriptor
	*/

	SOCKET clientSocket = INVALID_SOCKET;

	if (serverType == ServerType::TCP) clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else if (serverType == ServerType::UDP) clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (clientSocket == INVALID_SOCKET)
	{
		int errCode = WSAGetLastError();
		cout << "Socket ErrorCode: " << errCode << '\n';
		return 0;
	}

	// 2. IP, PORT 설정
	//char IP[] = "127.0.0.1";
	//char IP[] = "192.168.10.123";
	char IP[] = "192.168.10.101";
	//u_short PORT = 1998; // seating buck
	//u_short PORT = 1997; // udp
	u_short PORT = 1996; // tcp

	SOCKADDR_IN serverAddr; // IPv4
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &serverAddr.sin_addr);

	cout << IP << " " << PORT << ", " << serverAddr.sin_port << '\n';

	/*
		host to network short
		Little-Endian vs Big-Endian
		little: [0x78][0x56][0x34][0x12]
		big: [0x12][0x34][0x56][0x78] -> network 표준
	*/

	if (serverType == ServerType::TCP)
	{
		// 3. Socket 연결
		if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Socket ErrorCode: " << errCode << '\n';
			return 0;
		}
	}
	else if (serverType == ServerType::UDP)
	{
		if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Connect ErrorCode: " << errCode << '\n';
			return 0;
		}
	}

	cout << "Connected To Server!" << '\n';

	// 4. 데이터 송수신
	vector<thread> clientWorkers;
	clientWorkers.emplace_back(RawRecv, ref(clientSocket), ref(serverType));
	clientWorkers.emplace_back(RawSend, ref(clientSocket), ref(serverType), ref(serverAddr));

	// 5. Socket 종료
	for (auto& worker : clientWorkers)
	{
		if (worker.joinable())
			worker.join();
	}
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}

