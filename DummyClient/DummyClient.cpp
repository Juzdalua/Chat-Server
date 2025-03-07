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

void SendUDP(SOCKET& clientSocket, SOCKADDR_IN& serverAddr)
{
	while (true)
	{
		cout << "데이터를 입력하세요: ";
		string s;
		cin >> s;

		char sendBuffer[4096] = "";
		memcpy(sendBuffer, s.c_str(), s.size()); // 문자열 크기를 사용하여 복사

		// UDP일 때는 sendto() 사용
		int resultCode = sendto(clientSocket, sendBuffer, s.size(), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}
		cout << "[SEND] " << s;
		cout << "[SEND LEN] " << s.size();
	}
}

void SendUDPPacket(SOCKET& clientSocket, SOCKADDR_IN& serverAddr) {
	while (true) {
		this_thread::sleep_for(1s);
		unsigned char buffer[17];

		unsigned short sNetVersion = 2025;
		unsigned char bSize = 17;  // bSize를 17로 설정
		unsigned short sMask = 1;

		unsigned __int32 status = 2;
		float steerAngle = 1.2f;
		float steerAngleRate = 2.3f;

		memcpy(buffer, &sNetVersion, sizeof(unsigned short));        // 0~1 바이트에 sNetVersion
		memcpy(buffer + 2, &sMask, sizeof(unsigned short));                // 2~3 바이트에 sMask
		memcpy(buffer + 4, &bSize, sizeof(unsigned char));               // 4~5 바이트에 bSize
		memcpy(buffer + 5, &status, sizeof(unsigned __int32));             // 5~8 바이트에 status
		memcpy(buffer + 9, &steerAngle, sizeof(float));     // 9~12 바이트에 steerAngle
		memcpy(buffer + 13, &steerAngleRate, sizeof(float));  // 13~16 바이트에 steerAngleRate

		int resultCode = sendto(clientSocket, (const char*)buffer, 17, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		unsigned short sNetVersion1 = *reinterpret_cast<const unsigned short*>(&buffer[0]);
		short sMask1 = *reinterpret_cast<const short*>(&buffer[2]);
		unsigned char bSize1 = buffer[4];


		cout << "[SEND] ";
		cout << " sNetVersion1: "<< sNetVersion1;
		cout << " bSize1: "<< (int)bSize1;
		cout << " sMask1: " << sMask1;
		cout << '\n';
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

void RecvUDP(SOCKET& clientSocket, ServerType& serverType)
{
	char recvBuffer[4096]; // 수신 버퍼

	while (true)
	{
		int recvLen = 0;

		if (clientSocket == INVALID_SOCKET)
		{
			cout << "INVALID SOCKET" << '\n';
			return;
		}

		// UDP에서는 recvfrom() 사용
		SOCKADDR_IN senderAddr;
		memset(&senderAddr, 0, sizeof(senderAddr));  // senderAddr 초기화
		int senderAddrLen = sizeof(senderAddr);
		recvLen = recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&senderAddr, &senderAddrLen);

		if (recvLen <= 0)
		{
			int error = WSAGetLastError();
			std::cout << "Receive Error: " << error << '\n';
			this_thread::sleep_for(5s);
			return;
		}

		// 수신된 데이터의 길이만큼 출력
		recvBuffer[recvLen] = '\0';  // Null-terminate the string
		cout << "RecvLen: " << recvLen << '\n';
		cout << "[RECV] : " << recvBuffer << '\n';
	}
}

void RecvHandleUDP(SOCKET& clientSocket, ServerType& serverType)
{
	char recvBuffer[4096]; // 수신 버퍼

	while (true)
	{
		int recvLen = 0;

		if (clientSocket == INVALID_SOCKET)
		{
			std::cout << "INVALID SOCKET" << '\n';
			return;
		}

		// UDP에서는 recvfrom() 사용
		SOCKADDR_IN senderAddr;
		memset(&senderAddr, 0, sizeof(senderAddr));  // senderAddr 초기화
		int senderAddrLen = sizeof(senderAddr);
		recvLen = recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&senderAddr, &senderAddrLen);

		if (recvLen <= 0)
		{
			int error = WSAGetLastError();
			std::cout << "Receive Error: " << error << '\n';
			this_thread::sleep_for(5s);
			return;
		}

		// 패킷 분해하기
		if (recvLen >= 33)  // 패킷 길이가 최소 49바이트 이상이어야 함
		{
			// 헤더 데이터
			unsigned short sNetVersion;
			unsigned short sMask;
			unsigned char bSize;

			std::memcpy(&sNetVersion, recvBuffer, sizeof(unsigned short));           // 0~1 바이트
			std::memcpy(&sMask, recvBuffer + 2, sizeof(unsigned short));             // 2~3 바이트
			std::memcpy(&bSize, recvBuffer + 4, sizeof(unsigned char));             // 4~5 바이트

			// 데이터 필드
			unsigned int simState;
			float velocity, wheelAngleVelocityLF, wheelAngleVelocityRF;
			float wheelAngleVelocityLB, wheelAngleVelocityRB, targetAngle;

			std::memcpy(&simState, recvBuffer + 5, sizeof(unsigned int));            // 5~8 바이트
			std::memcpy(&velocity, recvBuffer + 9, sizeof(float));                  // 9~12 바이트
			std::memcpy(&wheelAngleVelocityLF, recvBuffer + 13, sizeof(float));    // 13~16 바이트
			std::memcpy(&wheelAngleVelocityRF, recvBuffer + 17, sizeof(float));    // 17~20 바이트
			std::memcpy(&wheelAngleVelocityLB, recvBuffer + 21, sizeof(float));    // 21~24 바이트
			std::memcpy(&wheelAngleVelocityRB, recvBuffer + 25, sizeof(float));    // 25~28 바이트
			std::memcpy(&targetAngle, recvBuffer + 29, sizeof(float));             // 29~32 바이트

			// 로깅 출력
			std::cout << "[RECV] Header:" << std::endl;
			std::cout << "  sNetVersion: " << sNetVersion << std::endl;
			std::cout << "  sMask: " << sMask << std::endl;
			std::cout << "  bSize: " << (int)bSize << std::endl;

			std::cout << "[RECV] Data:" << std::endl;
			std::cout << "  simState: " << simState << std::endl;
			std::cout << "  velocity: " << velocity << std::endl;
			std::cout << "  wheelAngleVelocityLF: " << wheelAngleVelocityLF << std::endl;
			std::cout << "  wheelAngleVelocityRF: " << wheelAngleVelocityRF << std::endl;
			std::cout << "  wheelAngleVelocityLB: " << wheelAngleVelocityLB << std::endl;
			std::cout << "  wheelAngleVelocityRB: " << wheelAngleVelocityRB << std::endl;
			std::cout << "  targetAngle: " << targetAngle << std::endl;
			std::cout << std::endl;
		}
		else
		{
			std::cout << "Received packet is too short: " << recvLen << " bytes." << std::endl;
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

	//ServerType serverType = ServerType::TCP;
	ServerType serverType = ServerType::UDP;

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
	char HOST_IP[] = "192.168.10.101";
	u_short HOST_PORT = 2100;
// 
	//char SERVER_IP[] = "127.0.0.1";
	//char SERVER_IP[] = "192.168.10.123";
	char SERVER_IP[] = "192.168.10.101";
	//u_short SERVER_PORT = 1998; // seating buck
	//u_short SERVER_PORT = 1997; // udp
	//u_short SERVER_PORT = 1996; // tcp
	u_short SERVER_PORT = 2000;

	SOCKADDR_IN hostAddr; // IPv4
	memset(&hostAddr, 0, sizeof(hostAddr));
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_port = htons(HOST_PORT);
	inet_pton(AF_INET, HOST_IP, &hostAddr.sin_addr);

	// 바인딩 (내 IP와 포트 사용)
	if (::bind(clientSocket, (struct sockaddr*)&hostAddr, sizeof(hostAddr)) == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		std::cerr << "Bind failed! "<<errorCode << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN serverAddr; // IPv4
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

	cout << HOST_IP << " " << HOST_PORT << ", " << serverAddr.sin_port << '\n';

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
		/*if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			cout << "Connect ErrorCode: " << errCode << '\n';
			return 0;
		}*/
	}

	cout << "Connected To Server!" << '\n';

	// 4. 데이터 송수신
	vector<thread> clientWorkers;
	//clientWorkers.emplace_back(RawRecv, ref(clientSocket), ref(serverType));
	//clientWorkers.emplace_back(RawSend, ref(clientSocket), ref(serverType), ref(serverAddr));
	//clientWorkers.emplace_back(RecvUDP, ref(clientSocket), ref(serverType));
	clientWorkers.emplace_back(RecvHandleUDP, ref(clientSocket), ref(serverType));
	//clientWorkers.emplace_back(SendUDP, ref(clientSocket), ref(serverAddr));
	clientWorkers.emplace_back(SendUDPPacket, ref(clientSocket), ref(serverAddr));

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

