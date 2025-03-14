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

#pragma pack(push,1)
struct SendPacketHeader
{
	unsigned short sNetVersion;
	short sMask;
	unsigned char bSize;
};
#pragma pack(pop)

#pragma pack(push,1)
struct SteerPacket {
	unsigned __int32 status;
	float steerAngle;
	float steerAngleRate;
};
#pragma pack(pop)

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

void SendUDPHandlePacket(SOCKET& clientSocket, SOCKADDR_IN& serverAddr) {
	while (true) {
		this_thread::sleep_for(1s);
		std::vector<unsigned char> buffer;
		const int BUFFER_SIZE = sizeof(SendPacketHeader) + sizeof(SteerPacket);
		buffer.resize(BUFFER_SIZE);

		SendPacketHeader sendPacketHeader = { 0 };
		sendPacketHeader.sNetVersion = 2025;
		sendPacketHeader.bSize = BUFFER_SIZE;
		sendPacketHeader.sMask = 0x0001;

		SteerPacket steerPacket = { 0 };
		steerPacket.status = 2;
		steerPacket.steerAngle = 1.2f;
		steerPacket.steerAngleRate = 2.3f;

		std::memcpy(buffer.data(), &sendPacketHeader, sizeof(SendPacketHeader));
		std::memcpy(buffer.data() + sizeof(SendPacketHeader), &steerPacket, sizeof(SteerPacket));

		int resultCode = sendto(clientSocket, (const char*)buffer.data(), BUFFER_SIZE, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		cout << "[SEND] ";
		cout << " sNetVersion: "<< sendPacketHeader.sNetVersion;
		cout << " bSize: "<< (int)sendPacketHeader.bSize;
		cout << " sMask: " << sendPacketHeader.sMask;
		cout << '\n';
	}
}

void SendUDPCabinControlPacket(SOCKET& clientSocket, SOCKADDR_IN& serverAddr) {
	const int BUFFER_SIZE = 19;
	while (true) {
		this_thread::sleep_for(1s);

		unsigned char buffer[BUFFER_SIZE];
		unsigned short sNetVersion = 2025;
		unsigned char bSize = BUFFER_SIZE;  // bSize를 17로 설정
		unsigned short sMask = 1;

		unsigned __int32 status = 2;
		float carHeight = 1.2f;
		float carWidth = 2.3f;
		float seatWidth = 2.3f;

		memcpy(buffer, &sNetVersion, sizeof(unsigned short));     
		memcpy(buffer + 2, &sMask, sizeof(unsigned short));                
		memcpy(buffer + 4, &bSize, sizeof(unsigned char));           
		memcpy(buffer + 5, &status, sizeof(unsigned __int32));       
		memcpy(buffer + 7, &carHeight, sizeof(float));    
		memcpy(buffer + 11, &carWidth, sizeof(float));  
		memcpy(buffer + 15, &seatWidth, sizeof(float)); 

		int resultCode = sendto(clientSocket, (const char*)buffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		unsigned short sNetVersion1 = *reinterpret_cast<const unsigned short*>(&buffer[0]);
		short sMask1 = *reinterpret_cast<const short*>(&buffer[2]);
		unsigned char bSize1 = buffer[4];


		cout << "[SEND] ";
		cout << " sNetVersion1: " << sNetVersion1;
		cout << " bSize1: " << (int)bSize1;
		cout << " sMask1: " << sMask1;
		cout << '\n';
	}
}

void SendUDPCabinSwitchPacket(SOCKET& clientSocket, SOCKADDR_IN& serverAddr) {
	const int BUFFER_SIZE = 62;
	while (true) {
		this_thread::sleep_for(1s);
		unsigned char buffer[BUFFER_SIZE];

		unsigned short sNetVersion = 2025;
		unsigned char bSize = BUFFER_SIZE;  // bSize를 17로 설정
		unsigned short sMask = 1;

		unsigned char GearTriger = 1;
		unsigned char GearP = 2;
		unsigned char Left_Paddle_Shift = 3;
		unsigned char Right_Paddle_Shift = 4;
		unsigned char Crs = 5;
		unsigned char voice = 6;
		unsigned char phone = 7;
		unsigned char mode = 8;
		unsigned char modeUp = 9;
		unsigned char modeDown = 10;
		unsigned char volumeMute = 11;
		unsigned char volumeWheel = 12;
		unsigned char Menu = 13;
		unsigned char MenuWheelbtn = 14;
		unsigned char Menuwheel = 15;
		unsigned char bookmark = 16;
		unsigned char Lamp_TrnSigLftSwSta = 17;
		unsigned char Lamp_TrnSigRtSwSta = 18;
		unsigned char Light = 19;
		unsigned char Lamp_HdLmpHiSwSta1 = 20;
		unsigned char Lamp_HdLmpHiSwSta2 = 21;
		unsigned char Wiper_FrWiperMist = 22;
		unsigned char Wiper_FrWiperWshSwSta = 23;
		unsigned char Wiper_FrWiperWshSwSta2 = 24;
		unsigned char Wiper_RrWiperWshSwSta = 25;
		unsigned char NGB = 26;
		unsigned char DriveModeSw = 27;
		unsigned char LeftN = 28;
		unsigned char RightN = 29;
		unsigned char HOD_Dir_Status = 30;
		unsigned char FWasher = 31;
		unsigned char Parking = 32;
		unsigned char SeatBelt1 = 33;
		unsigned char SeatBelt2 = 34;
		unsigned char EMG = 35;
		unsigned char Key = 36;
		unsigned char Trunk = 37;
		unsigned char VDC = 38;
		unsigned char Booster = 39;
		unsigned char Plus = 40;
		unsigned char Right = 41;
		unsigned char Minus = 42;
		unsigned char Voice = 43;
		unsigned char OK = 44;
		unsigned char Left = 45;
		unsigned char Phone = 46;
		unsigned char PlusSet = 47;
		unsigned char Distance = 48;
		unsigned char MinusSet = 49;
		unsigned char LFA = 50;
		unsigned char SCC = 51;
		unsigned char CC = 52;
		unsigned char DriveMode = 53;
		unsigned char LightHeight = 54;
		unsigned char ACCpedal = 55;
		unsigned char Brakepedal = 56;
		unsigned char bMask = 57;

		memcpy(buffer, &sNetVersion, sizeof(unsigned short));       // 0~1 바이트에 sNetVersion
		memcpy(buffer + 2, &sMask, sizeof(unsigned short));          // 2~3 바이트에 sMask
		memcpy(buffer + 4, &bSize, sizeof(unsigned char));           // 4 바이트에 bSize

		memcpy(buffer + 5, &GearTriger, sizeof(unsigned char));
		memcpy(buffer + 6, &GearP, sizeof(unsigned char));
		memcpy(buffer + 7, &Left_Paddle_Shift, sizeof(unsigned char));
		memcpy(buffer + 8, &Right_Paddle_Shift, sizeof(unsigned char));
		memcpy(buffer + 9, &Crs, sizeof(unsigned char));
		memcpy(buffer + 10, &voice, sizeof(unsigned char));
		memcpy(buffer + 11, &phone, sizeof(unsigned char));
		memcpy(buffer + 12, &mode, sizeof(unsigned char));
		memcpy(buffer + 13, &modeUp, sizeof(unsigned char));
		memcpy(buffer + 14, &modeDown, sizeof(unsigned char));
		memcpy(buffer + 15, &volumeMute, sizeof(unsigned char));
		memcpy(buffer + 16, &volumeWheel, sizeof(unsigned char));
		memcpy(buffer + 17, &Menu, sizeof(unsigned char));
		memcpy(buffer + 18, &MenuWheelbtn, sizeof(unsigned char));
		memcpy(buffer + 19, &Menuwheel, sizeof(unsigned char));
		memcpy(buffer + 20, &bookmark, sizeof(unsigned char));
		memcpy(buffer + 21, &Lamp_TrnSigLftSwSta, sizeof(unsigned char));
		memcpy(buffer + 22, &Lamp_TrnSigRtSwSta, sizeof(unsigned char));
		memcpy(buffer + 23, &Light, sizeof(unsigned char));
		memcpy(buffer + 24, &Lamp_HdLmpHiSwSta1, sizeof(unsigned char));
		memcpy(buffer + 25, &Lamp_HdLmpHiSwSta2, sizeof(unsigned char));
		memcpy(buffer + 26, &Wiper_FrWiperMist, sizeof(unsigned char));
		memcpy(buffer + 27, &Wiper_FrWiperWshSwSta, sizeof(unsigned char));
		memcpy(buffer + 28, &Wiper_FrWiperWshSwSta2, sizeof(unsigned char));
		memcpy(buffer + 29, &Wiper_RrWiperWshSwSta, sizeof(unsigned char));
		memcpy(buffer + 30, &NGB, sizeof(unsigned char));
		memcpy(buffer + 31, &DriveModeSw, sizeof(unsigned char));
		memcpy(buffer + 32, &LeftN, sizeof(unsigned char));
		memcpy(buffer + 33, &RightN, sizeof(unsigned char));
		memcpy(buffer + 34, &HOD_Dir_Status, sizeof(unsigned char));
		memcpy(buffer + 35, &FWasher, sizeof(unsigned char));
		memcpy(buffer + 36, &Parking, sizeof(unsigned char));
		memcpy(buffer + 37, &SeatBelt1, sizeof(unsigned char));
		memcpy(buffer + 38, &SeatBelt2, sizeof(unsigned char));
		memcpy(buffer + 39, &EMG, sizeof(unsigned char));
		memcpy(buffer + 40, &Key, sizeof(unsigned char));
		memcpy(buffer + 41, &Trunk, sizeof(unsigned char));
		memcpy(buffer + 42, &VDC, sizeof(unsigned char));
		memcpy(buffer + 43, &Booster, sizeof(unsigned char));
		memcpy(buffer + 44, &Plus, sizeof(unsigned char));
		memcpy(buffer + 45, &Right, sizeof(unsigned char));
		memcpy(buffer + 46, &Minus, sizeof(unsigned char));
		memcpy(buffer + 47, &Voice, sizeof(unsigned char));
		memcpy(buffer + 48, &OK, sizeof(unsigned char));
		memcpy(buffer + 49, &Left, sizeof(unsigned char));
		memcpy(buffer + 50, &Phone, sizeof(unsigned char));
		memcpy(buffer + 51, &PlusSet, sizeof(unsigned char));
		memcpy(buffer + 52, &Distance, sizeof(unsigned char));
		memcpy(buffer + 53, &MinusSet, sizeof(unsigned char));
		memcpy(buffer + 54, &LFA, sizeof(unsigned char));
		memcpy(buffer + 55, &SCC, sizeof(unsigned char));
		memcpy(buffer + 56, &CC, sizeof(unsigned char));
		memcpy(buffer + 57, &DriveMode, sizeof(unsigned char));
		memcpy(buffer + 58, &LightHeight, sizeof(unsigned char));
		memcpy(buffer + 59, &ACCpedal, sizeof(unsigned char));
		memcpy(buffer + 60, &Brakepedal, sizeof(unsigned char));
		memcpy(buffer + 61, &bMask, sizeof(unsigned char));


		int resultCode = sendto(clientSocket, (const char*)buffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		unsigned short sNetVersion1 = *reinterpret_cast<const unsigned short*>(&buffer[0]);
		short sMask1 = *reinterpret_cast<const short*>(&buffer[2]);
		unsigned char bSize1 = buffer[4];


		cout << "[SEND] ";
		cout << " sNetVersion1: " << sNetVersion1;
		cout << " bSize1: " << (int)bSize1;
		cout << " sMask1: " << sMask1;

		cout << '\n';
	}
}

void SendUDPMotionPacket(SOCKET& clientSocket, SOCKADDR_IN& serverAddr) {
	const int BUFFER_SIZE = 137;
	while (true) {
		this_thread::sleep_for(1s);
		unsigned char buffer[BUFFER_SIZE];

		unsigned short sNetVersion = 2025;
		unsigned char bSize = BUFFER_SIZE;  // bSize를 17로 설정
		unsigned short sMask = 1;

		uint32_t FrameCounter = 1234;
		uint32_t motionStatus = 1;
		uint32_t errorLevel = 2;
		uint32_t errorCode = 0;
		uint32_t ioInfo = 3;

		float xPosition = 12.34f, yPosition = 56.78f, zPosition = 90.12f;
		float yawPosition = 1.23f, pitchPosition = 4.56f, rollPosition = 7.89f;
		float xSpeed = 10.11f, ySpeed = 12.13f, zSpeed = 14.15f;
		float yawSpeed = 1.16f, pitchSpeed = 1.17f, rollSpeed = 1.18f;
		float xAcc = 0.19f, yAcc = 0.20f, zAcc = 0.21f;
		float yawAcc = 0.22f, pitchAcc = 0.23f, rollAcc = 0.24f;
		float actuator1Length = 1.25f, actuator2Length = 1.26f, actuator3Length = 1.27f;
		float actuator4Length = 1.28f, actuator5Length = 1.29f, actuator6Length = 1.30f;
		float analogInput1 = 0.31f, analogInput2 = 0.32f, analogInput3 = 0.33f, analogInput4 = 0.34f;

		memcpy(buffer, &sNetVersion, sizeof(unsigned short));       // 0~1 바이트에 sNetVersion
		memcpy(buffer + 2, &sMask, sizeof(unsigned short));          // 2~3 바이트에 sMask
		memcpy(buffer + 4, &bSize, sizeof(unsigned char));           // 4 바이트에 bSize

		size_t offset = 5;

		// uint32_t 데이터 추가 (FrameCounter, Motion Status, Error Level, Error Code, IO Info)
		memcpy(buffer + offset, &FrameCounter, sizeof(FrameCounter)); offset += sizeof(FrameCounter);
		memcpy(buffer + offset, &motionStatus, sizeof(motionStatus)); offset += sizeof(motionStatus);
		memcpy(buffer + offset, &errorLevel, sizeof(errorLevel)); offset += sizeof(errorLevel);
		memcpy(buffer + offset, &errorCode, sizeof(errorCode)); offset += sizeof(errorCode);
		memcpy(buffer + offset, &ioInfo, sizeof(ioInfo)); offset += sizeof(ioInfo);

		// float32 데이터 추가 (위치, 속도, 가속도 등)
		memcpy(buffer + offset, &xPosition, sizeof(xPosition)); offset += sizeof(xPosition);
		memcpy(buffer + offset, &yPosition, sizeof(yPosition)); offset += sizeof(yPosition);
		memcpy(buffer + offset, &zPosition, sizeof(zPosition)); offset += sizeof(zPosition);
		memcpy(buffer + offset, &yawPosition, sizeof(yawPosition)); offset += sizeof(yawPosition);
		memcpy(buffer + offset, &pitchPosition, sizeof(pitchPosition)); offset += sizeof(pitchPosition);
		memcpy(buffer + offset, &rollPosition, sizeof(rollPosition)); offset += sizeof(rollPosition);

		memcpy(buffer + offset, &xSpeed, sizeof(xSpeed)); offset += sizeof(xSpeed);
		memcpy(buffer + offset, &ySpeed, sizeof(ySpeed)); offset += sizeof(ySpeed);
		memcpy(buffer + offset, &zSpeed, sizeof(zSpeed)); offset += sizeof(zSpeed);
		memcpy(buffer + offset, &yawSpeed, sizeof(yawSpeed)); offset += sizeof(yawSpeed);
		memcpy(buffer + offset, &pitchSpeed, sizeof(pitchSpeed)); offset += sizeof(pitchSpeed);
		memcpy(buffer + offset, &rollSpeed, sizeof(rollSpeed)); offset += sizeof(rollSpeed);

		memcpy(buffer + offset, &xAcc, sizeof(xAcc)); offset += sizeof(xAcc);
		memcpy(buffer + offset, &yAcc, sizeof(yAcc)); offset += sizeof(yAcc);
		memcpy(buffer + offset, &zAcc, sizeof(zAcc)); offset += sizeof(zAcc);
		memcpy(buffer + offset, &yawAcc, sizeof(yawAcc)); offset += sizeof(yawAcc);
		memcpy(buffer + offset, &pitchAcc, sizeof(pitchAcc)); offset += sizeof(pitchAcc);
		memcpy(buffer + offset, &rollAcc, sizeof(rollAcc)); offset += sizeof(rollAcc);

		// 액추에이터 길이 값들
		memcpy(buffer + offset, &actuator1Length, sizeof(actuator1Length)); offset += sizeof(actuator1Length);
		memcpy(buffer + offset, &actuator2Length, sizeof(actuator2Length)); offset += sizeof(actuator2Length);
		memcpy(buffer + offset, &actuator3Length, sizeof(actuator3Length)); offset += sizeof(actuator3Length);
		memcpy(buffer + offset, &actuator4Length, sizeof(actuator4Length)); offset += sizeof(actuator4Length);
		memcpy(buffer + offset, &actuator5Length, sizeof(actuator5Length)); offset += sizeof(actuator5Length);
		memcpy(buffer + offset, &actuator6Length, sizeof(actuator6Length)); offset += sizeof(actuator6Length);

		// 아날로그 입력 값들
		memcpy(buffer + offset, &analogInput1, sizeof(analogInput1)); offset += sizeof(analogInput1);
		memcpy(buffer + offset, &analogInput2, sizeof(analogInput2)); offset += sizeof(analogInput2);
		memcpy(buffer + offset, &analogInput3, sizeof(analogInput3)); offset += sizeof(analogInput3);
		memcpy(buffer + offset, &analogInput4, sizeof(analogInput4)); offset += sizeof(analogInput4);


		int resultCode = sendto(clientSocket, (const char*)buffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (resultCode == SOCKET_ERROR) {
			int errCode = WSAGetLastError();
			cout << "Send ErrorCode: " << errCode << '\n';
			return;
		}

		// 받은 데이터 출력
		unsigned short sNetVersion1 = *reinterpret_cast<const unsigned short*>(&buffer[0]);
		short sMask1 = *reinterpret_cast<const short*>(&buffer[2]);
		unsigned char bSize1 = buffer[4];

		uint32_t FrameCounter1 = *reinterpret_cast<const uint32_t*>(&buffer[5]);
		uint32_t motionStatus1 = *reinterpret_cast<const uint32_t*>(&buffer[9]);
		uint32_t errorLevel1 = *reinterpret_cast<const uint32_t*>(&buffer[13]);
		uint32_t errorCode1 = *reinterpret_cast<const uint32_t*>(&buffer[17]);
		uint32_t ioInfo1 = *reinterpret_cast<const uint32_t*>(&buffer[21]);

		float xPosition1 = *reinterpret_cast<const float*>(&buffer[25]);
		float yPosition1 = *reinterpret_cast<const float*>(&buffer[29]);
		float zPosition1 = *reinterpret_cast<const float*>(&buffer[33]);
		float yawPosition1 = *reinterpret_cast<const float*>(&buffer[37]);
		float pitchPosition1 = *reinterpret_cast<const float*>(&buffer[41]);
		float rollPosition1 = *reinterpret_cast<const float*>(&buffer[45]);

		float xSpeed1 = *reinterpret_cast<const float*>(&buffer[49]);
		float ySpeed1 = *reinterpret_cast<const float*>(&buffer[53]);
		float zSpeed1 = *reinterpret_cast<const float*>(&buffer[57]);
		float yawSpeed1 = *reinterpret_cast<const float*>(&buffer[61]);
		float pitchSpeed1 = *reinterpret_cast<const float*>(&buffer[65]);
		float rollSpeed1 = *reinterpret_cast<const float*>(&buffer[69]);

		float xAcc1 = *reinterpret_cast<const float*>(&buffer[73]);
		float yAcc1 = *reinterpret_cast<const float*>(&buffer[77]);
		float zAcc1 = *reinterpret_cast<const float*>(&buffer[81]);
		float yawAcc1 = *reinterpret_cast<const float*>(&buffer[85]);
		float pitchAcc1 = *reinterpret_cast<const float*>(&buffer[89]);
		float rollAcc1 = *reinterpret_cast<const float*>(&buffer[93]);

		float actuator1Length1 = *reinterpret_cast<const float*>(&buffer[97]);
		float actuator2Length1 = *reinterpret_cast<const float*>(&buffer[101]);
		float actuator3Length1 = *reinterpret_cast<const float*>(&buffer[105]);
		float actuator4Length1 = *reinterpret_cast<const float*>(&buffer[109]);
		float actuator5Length1 = *reinterpret_cast<const float*>(&buffer[113]);
		float actuator6Length1 = *reinterpret_cast<const float*>(&buffer[117]);

		float analogInput1_1 = *reinterpret_cast<const float*>(&buffer[121]);
		float analogInput2_1 = *reinterpret_cast<const float*>(&buffer[125]);
		float analogInput3_1 = *reinterpret_cast<const float*>(&buffer[129]);
		float analogInput4_1 = *reinterpret_cast<const float*>(&buffer[133]);

		cout << "[SEND] ";
		cout << "sNetVersion1: " << sNetVersion1 << " ";
		cout << "sMask1: " << sMask1 << " ";
		cout << "bSize1: " << (int)bSize1 << " ";
		cout << "FrameCounter1: " << FrameCounter1 << " ";
		cout << "motionStatus1: " << motionStatus1 << " ";
		cout << "errorLevel1: " << errorLevel1 << " ";
		cout << "errorCode1: " << errorCode1 << " ";
		cout << "ioInfo1: " << ioInfo1 << " ";

		cout << "xPosition1: " << xPosition1 << " ";
		cout << "yPosition1: " << yPosition1 << " ";
		cout << "zPosition1: " << zPosition1 << " ";
		cout << "yawPosition1: " << yawPosition1 << " ";
		cout << "pitchPosition1: " << pitchPosition1 << " ";
		cout << "rollPosition1: " << rollPosition1 << " ";

		cout << "xSpeed1: " << xSpeed1 << " ";
		cout << "ySpeed1: " << ySpeed1 << " ";
		cout << "zSpeed1: " << zSpeed1 << " ";
		cout << "yawSpeed1: " << yawSpeed1 << " ";
		cout << "pitchSpeed1: " << pitchSpeed1 << " ";
		cout << "rollSpeed1: " << rollSpeed1 << " ";

		cout << "xAcc1: " << xAcc1 << " ";
		cout << "yAcc1: " << yAcc1 << " ";
		cout << "zAcc1: " << zAcc1 << " ";
		cout << "yawAcc1: " << yawAcc1 << " ";
		cout << "pitchAcc1: " << pitchAcc1 << " ";
		cout << "rollAcc1: " << rollAcc1 << " ";

		cout << "actuator1Length1: " << actuator1Length1 << " ";
		cout << "actuator2Length1: " << actuator2Length1 << " ";
		cout << "actuator3Length1: " << actuator3Length1 << " ";
		cout << "actuator4Length1: " << actuator4Length1 << " ";
		cout << "actuator5Length1: " << actuator5Length1 << " ";
		cout << "actuator6Length1: " << actuator6Length1 << " ";

		cout << "analogInput1_1: " << analogInput1_1 << " ";
		cout << "analogInput2_1: " << analogInput2_1 << " ";
		cout << "analogInput3_1: " << analogInput3_1 << " ";
		cout << "analogInput4_1: " << analogInput4_1 << endl;



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

		std::cout << "[RECV] LEN = " << recvLen << '\n';
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
	
	clientWorkers.emplace_back(SendUDPHandlePacket, ref(clientSocket), ref(serverAddr));
	//clientWorkers.emplace_back(SendUDPCabinControlPacket, ref(clientSocket), ref(serverAddr));
	//clientWorkers.emplace_back(SendUDPCabinSwitchPacket, ref(clientSocket), ref(serverAddr));
	//clientWorkers.emplace_back(SendUDPMotionPacket, ref(clientSocket), ref(serverAddr));

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

