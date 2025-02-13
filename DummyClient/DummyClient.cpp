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

#pragma pack(push,1)
struct PacketHeader
{
	UINT size; // 패킷 size
	UINT id; // 프로토콜 ID (ex 1=로그인, 2=이동요청)
};
#pragma pack(pop)

void EchoClient(SOCKET& clientSocket)
{
	while (true)
	{
		char sendBuffer[100] = "";
		string input;
		cin >> input;
		for (int i = 0; i < input.size(); i++)
		{
			sendBuffer[i] = input[i];
		}

		int resultCode = send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		// Echo Receiver
		char recvBuffer[1000];
		int recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLen <= 0)
		{
			int errCode = WSAGetLastError();
			cout << "Recv ErrorCode: " << errCode << '\n';
			return;
		}

		cout << "Recv Data! Len = " << recvLen << '\n';
		cout << "Recv Data! Data = " << recvBuffer << '\n';
	}
}

void RawSend(SOCKET& clientSocket)
{
	while (true)
	{
		string s;
		cin >> s;

		json jsonData = {
			{"data", s},
		};

		std::string jsonString = jsonData.dump(); // JSON -> String
		UINT jsonSize = static_cast<UINT>(jsonString.size());
		int totalPacketSize = jsonSize + sizeof(PacketHeader);

		//UINT id = 5000U;
		UINT id = 123U;
		PacketHeader sendHeader = { 0 };
		sendHeader.id = id;
		sendHeader.size = sizeof(PacketHeader) + jsonString.size();
		cout << "[SEND] ID -> " << sendHeader.id << ", SIZE -> " << sendHeader.size << '\n';

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, &sendHeader, sizeof(PacketHeader));
		memcpy(sendBuffer + sizeof(PacketHeader), jsonString.data(), jsonSize);

		int resultCode = send(clientSocket, sendBuffer, totalPacketSize, 0);
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}
	}
}

void RawSendVelocity7000(SOCKET& clientSocket)
{
	while (true)
	{
		this_thread::sleep_for(1s);

		/*json jsonData = {
			{"drivingMode", 0},
			{"drivingMap" , 0},
			{"simRacingMap" , 0},
			{"weather" , 0},
			{"time" , 0},
			{"traffic" , 0},
			{"carSelection" , 0 },
			{"pageNum", 0},
		};*/

		json jsonData = {
			{"velocity", 16.32f},
		};

		std::string jsonString = jsonData.dump(); // JSON -> String
		UINT jsonSize = static_cast<UINT>(jsonString.size());
		int totalPacketSize = jsonSize + sizeof(PacketHeader);

		//UINT id = 5000U;
		UINT id = 7000U;
		PacketHeader sendHeader = { 0 };
		sendHeader.id = id;
		sendHeader.size = sizeof(PacketHeader) + jsonString.size();
		cout << "SEND => " << ", ID -> " << sendHeader.id << ", SIZE -> " << sendHeader.size << '\n';

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, &sendHeader, sizeof(PacketHeader));
		memcpy(sendBuffer + sizeof(PacketHeader), jsonString.data(), jsonSize);

		int resultCode = send(clientSocket, sendBuffer, totalPacketSize, 0);
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}
	}
}

void RawSendSet5000(SOCKET& clientSocket)
{
	while (true)
	{
		this_thread::sleep_for(1s);

		json jsonData = {
			{"drivingMode", 0},
			{"drivingMap" , 0},
			{"simRacingMap" , 0},
			{"weather" , 0},
			{"time" , 0},
			{"traffic" , 0},
			{"carSelection" , 0 },
			{"pageNum", 0},
		};

		std::string jsonString = jsonData.dump(); // JSON -> String
		UINT jsonSize = static_cast<UINT>(jsonString.size());
		int totalPacketSize = jsonSize + sizeof(PacketHeader);

		UINT id = 5000U;
		PacketHeader sendHeader = { 0 };
		sendHeader.id = id;
		sendHeader.size = sizeof(PacketHeader) + jsonString.size();
		cout << "SEND => " << ", ID -> " << sendHeader.id << ", SIZE -> " << sendHeader.size << '\n';

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, &sendHeader, sizeof(PacketHeader));
		memcpy(sendBuffer + sizeof(PacketHeader), jsonString.data(), jsonSize);

		int resultCode = send(clientSocket, sendBuffer, totalPacketSize, 0);
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}
	}
}

void RawSendTest(SOCKET& clientSocket)
{
	while (true)
	{
		this_thread::sleep_for(1s);

		json jsonData = {
			{"test1", 0},
			{"test2" , "abcd"},
			{"test3" , 'Z'},
		};

		std::string jsonString = jsonData.dump(); // JSON -> String
		UINT jsonSize = static_cast<UINT>(jsonString.size());
		int totalPacketSize = jsonSize + sizeof(PacketHeader);

		UINT id = 1231U;
		PacketHeader sendHeader = { 0 };
		sendHeader.id = id;
		sendHeader.size = sizeof(PacketHeader) + jsonString.size();
		cout << "SEND => " << ", ID -> " << sendHeader.id << ", SIZE -> " << sendHeader.size << '\n';

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, &sendHeader, sizeof(PacketHeader));
		memcpy(sendBuffer + sizeof(PacketHeader), jsonString.data(), jsonSize);

		int resultCode = send(clientSocket, sendBuffer, totalPacketSize, 0);
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}
	}
}

//void RawRecv(SOCKET& clientSocket)
//{
//	while (true)
//	{
//		// RECV
//		char recvBuffer[4096];
//		int recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
//		if (recvLen <= 0)
//		{
//			int errCode = WSAGetLastError();
//			cout << "Recv ErrorCode: " << errCode << '\n';
//			return;
//		}
//		int headerSize = sizeof(PacketHeader);
//		if (recvLen >= headerSize)
//		{
//			PacketHeader* header = reinterpret_cast<PacketHeader*>(recvBuffer);
//			cout << '\n' << "LEN => " << recvLen << ", ID -> " << header->id << ", SIZE -> " << header->size << '\n';
//
//			// JSON 데이터 추출
//			if (recvLen > headerSize)
//			{
//				const char* jsonData = recvBuffer + headerSize;
//				int jsonLen = recvLen - headerSize;
//
//				// JSON 데이터 출력 (NULL 종료 보장)
//				string jsonString(jsonData, jsonLen);
//				cout << "JSON Data: " << jsonString << '\n';
//			}
//		}
//	}
//}

void RawRecv(SOCKET& clientSocket)
{
	char recvBuffer[4096]; // 수신 버퍼
	int processLen = 0;    // 처리된 데이터 길이

	while (true)
	{
		// 데이터 수신
		int recvLen = recv(clientSocket, recvBuffer + processLen, sizeof(recvBuffer) - processLen, 0);
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
			cout << '\n' << "[RECV] ID -> " << header->id << ", SIZE -> " << header->size << ", LEN => " << recvLen << '\n';

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


void RawRecvOnlyId(SOCKET& clientSocket)
{
	char recvBuffer[4096]; // 수신 버퍼
	int processLen = 0;    // 처리된 데이터 길이

	while (true)
	{
		// 데이터 수신
		int recvLen = recv(clientSocket, recvBuffer + processLen, sizeof(recvBuffer) - processLen, 0);
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
			cout << "ID -> " << header->id << ", SIZE -> " << header->size << ", LEN => " << recvLen << '\n';

			if (header->size > sizeof(PacketHeader))
			{
				const char* payload = recvBuffer + sizeof(PacketHeader);
				int payloadSize = header->size - sizeof(PacketHeader);

				std::string jsonString(payload, payloadSize);
				//std::cout << "JSON Data: " << jsonString << '\n';
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

	/*
		af: Address Family (AF_INET = IPv4, AF_INTE6 = IPv6)
		type: TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
		protocol: 0
		return: descriptor
	*/
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int errCode = WSAGetLastError();
		cout << "Socket ErrorCode: " << errCode << '\n';
		return 0;
	}

	// 2. IP, PORT 설정
	char IP[] = "127.0.0.1";
	//char IP[] = "192.168.10.134";
	//char IP[] = "192.168.10.129";
	u_short PORT = 1998;

	SOCKADDR_IN serverAddr; // IPv4
	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;

	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); <- deprecated
	inet_pton(AF_INET, IP, &serverAddr.sin_addr);

	cout << IP << " " << PORT << '\n';

	serverAddr.sin_port = htons(PORT);
	/*
		host to network short
		Little-Endian vs Big-Endian
		little: [0x78][0x56][0x34][0x12]
		big: [0x12][0x34][0x56][0x78] -> network 표준
	*/

	// 3. Socket 연결
	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		cout << "Socket ErrorCode: " << errCode << '\n';
		return 0;
	}

	cout << "Connected To Server!" << '\n';

	// 4. 데이터 송수신
	vector<thread> clientWorkers;
	clientWorkers.emplace_back(RawRecv, ref(clientSocket));
	clientWorkers.emplace_back(RawSend, ref(clientSocket));
	//clientWorkers.emplace_back(RawRecvOnlyId, ref(clientSocket));
	//clientWorkers.emplace_back(RawSendSet5000, ref(clientSocket));
	//clientWorkers.emplace_back(RawSendVelocity7000, ref(clientSocket));
	//clientWorkers.emplace_back(RawSendTest, ref(clientSocket));

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

