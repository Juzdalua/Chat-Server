﻿#define WIN32_LEAN_AND_MEAN   
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
};


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

void RecvClient(SOCKET& clientSocket)
{
	while (true)
	{
		unsigned char buffer[4096]; // 데이터 버퍼를 unsigned char*로 변경
		int totalReceived = 0;

		// 1. 헤더 수신
		int headerSize = sizeof(PacketHeader);
		while (totalReceived < headerSize) {
			int received = recv(clientSocket, reinterpret_cast<char*>(buffer) + totalReceived, headerSize - totalReceived, 0);
			if (received <= 0) {
				cerr << "Header Recv Error or Connection Closed!" << '\n';
				return;
			}
			totalReceived += received;
		}

		// 2. 헤더 파싱
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		int totalPacketSize = header->size;  // 헤더에 정의된 전체 패킷 크기
		cout << "Recv Packet Size: " << totalPacketSize << '\n';

		// 3. 본문 수신
		totalReceived = 0;
		while (totalReceived < totalPacketSize - headerSize) {
			int received = recv(clientSocket, reinterpret_cast<char*>(buffer) + headerSize + totalReceived,
				totalPacketSize - headerSize - totalReceived, 0);
			if (received <= 0) {
				cerr << "Data Recv Error or Connection Closed!" << '\n';
				return;
			}
			totalReceived += received;
		}

		// 4. JSON 데이터 처리
		string jsonData(reinterpret_cast<char*>(buffer) + headerSize, totalPacketSize - headerSize); // 헤더 이후 데이터 추출
		try {
			json parsedData = json::parse(jsonData);
			cout << "Data: " << parsedData.dump(4) << '\n';
		}
		catch (exception& e) {
			cerr << "JSON Parsing Error: " << e.what() << '\n';
		}
	}
}

void RawSend(SOCKET& clientSocket)
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

void RawRecv(SOCKET& clientSocket)
{
	while (true)
	{
		// RECV
		char recvBuffer[4096];
		int recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLen <= 0)
		{
			int errCode = WSAGetLastError();
			cout << "Recv ErrorCode: " << errCode << '\n';
			return;
		}

		PacketHeader* header = reinterpret_cast<PacketHeader*>(recvBuffer);

		cout << '\n' << "LEN => " << recvLen << ", ID -> " << header->id << ", SIZE -> " << header->size << '\n';

		// JSON 데이터 추출
		int headerSize = sizeof(PacketHeader);
		if (recvLen > headerSize)
		{
			const char* jsonData = recvBuffer + headerSize;
			int jsonLen = recvLen - headerSize;

			// JSON 데이터 출력 (NULL 종료 보장)
			string jsonString(jsonData, jsonLen);
			cout << "JSON Data: " << jsonString << '\n';
		}
	}
}

void SendTestInput(SOCKET& clientSocket)
{
	int pageNum = 0;
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

	while (true)
	{
		int input;
		cin >> input;

		if (pageNum > 4) break;

		switch (pageNum)
		{
		case 0:
			if (input >= 1 && input <= 2) jsonData["drivingMode"] = input;
			else jsonData["drivingMode"] = 1;
			pageNum++;
			break;
		
		case 1:
			if (jsonData["drivingMode"] == 1)
			{
				if (input >= 1 && input <= 2) jsonData["drivingMap"] = input;
				else jsonData["drivingMap"] = 1;
				jsonData["simRacingMap"] = 0;
			}
			else if (jsonData["drivingMode"] == 2)
			{
				if (input >= 1 && input <= 4) jsonData["simRacingMap"] = input;
				else jsonData["simRacingMap"] = 1;
				jsonData["drivingMap"] = 0;
			}
			pageNum++;
			break;
		
		case 2:
			if (input >= 1 && input <= 2) jsonData["weather"] = input;
			else jsonData["weather"] = 1;
			pageNum++;
			break;

		case 3:
			int input2, input3;
			cin >> input2 >> input3;

			if (input >= 1 && input <= 2) jsonData["time"] = input;
			else jsonData["time"] = 1;

			if (input2 >= 1 && input2 <= 3) jsonData["traffic"] = input2;
			else jsonData["traffic"] = 1;

			if (input3 >= 1 && input3 <= 2) jsonData["carSelection"] = input3;
			else jsonData["carSelection"] = 1;
			pageNum++;
			break;
		}

		jsonData["pageNum"] = pageNum;

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
	//char IP[] = "127.0.0.1";
	char IP[] = "192.168.10.123";
	//char IP[] = "192.168.10.129";
	u_short PORT = 1998;

	SOCKADDR_IN serverAddr; // IPv4
	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;

	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); <- deprecated
	inet_pton(AF_INET, IP, &serverAddr.sin_addr);

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
	//EchoClient(clientSocket);
	//RecvClient(clientSocket);
	//RawRecv(clientSocket);

	vector<thread> clientWorkers;
	clientWorkers.emplace_back(RawRecv, ref(clientSocket));
	//clientWorkers.emplace_back(RawSend, ref(clientSocket));
	//clientWorkers.emplace_back(SendTestInput, ref(clientSocket));

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

