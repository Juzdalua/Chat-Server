#pragma once
#include <json.hpp>
#include <string>
#include "PacketQueue.h"
#include "Session.h"
#include "UDPServer.h"

//     __ _____ _____ _____
//  __|  |   __|     |   | |  
// |  |  |__   |  |  | | | |  
// |_____|_____|_____|_|___| 
using json = nlohmann::json;

enum : UINT
{
	///////////////////////////////////////////////////////////
	// LiveModApp�� ������ ������ 5000
	///////////////////////////////////////////////////////////
	PKT_SET_INFO = 5001, // ivi -> server -> ue

	///////////////////////////////////////////////////////////
	// ���ù��� ������ ������ 6000
	///////////////////////////////////////////////////////////
	PKT_SEATINGBUCK_BUTTON = 6001, // mdaq -> server -> ue : broadcast mdaq mfc program
	PKT_SEATINGBUCK_HANDLE = 6002, // broadcast inno mfc program

	///////////////////////////////////////////////////////////
	// �𸮾��� ������ ������ 7000
	///////////////////////////////////////////////////////////
	PKT_DRIVING_STATE = 7001, // ue -> server -> ivi : velocity

	PKT_AUTO_DRIVE = 7002,
	PKT_DONE_AUTO_DRIVE = 7003,
};

class ClientPacketHandler
{
public:
	static void HandlePacket(std::shared_ptr<PacketData> pkt);

	// ���� �� ������ �Է�
	static void HandleSetInfo(std::shared_ptr<PacketData> pkt);

	// MDAQ -> INNO Seatingbuck Button data
	static void HandleMDAQData(std::shared_ptr<PacketData> pkt);

	// UE -> Server
	static void HandleDrivingState(std::shared_ptr<PacketData> pkt);

	// ��������
	static void HandleAutoDrive(std::shared_ptr<PacketData> pkt);
	static void HandleDoneAutoDrive(std::shared_ptr<PacketData> pkt);

	// ���� ����
	static void Broadcast(PacketHeader& header, std::string& jsonString);

public:
	static ServerType _serverType;
	static shared_ptr<UDPServer> _udpServer;
};
