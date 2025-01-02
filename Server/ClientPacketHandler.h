#pragma once
#include <json.hpp>
#include <string>
#include "PacketQueue.h"
#include "Session.h"

//     __ _____ _____ _____
//  __|  |   __|     |   | |  
// |  |  |__   |  |  | | | |  
// |_____|_____|_____|_|___| 
using json = nlohmann::json;

enum : UINT
{
	///////////////////////////////////////////////////////////
	// LiveModApp이 보내는 데이터 5000
	///////////////////////////////////////////////////////////
	PKT_C_SET_INFO = 5000, // ivi -> server
	PKT_S_SET_INFO = 5001, // server -> ue

	PKT_C_DRIVING_STATUS = 5002,
	PKT_S_DRIVING_STATUS = 5003,

	///////////////////////////////////////////////////////////
	// 시팅벅이 보내는 데이터 6000
	///////////////////////////////////////////////////////////
	PKT_C_SEATINGBUCK_BUTTON = 6001, // recv from mdaq
	PKT_S_SEATINGBUCK_BUTTON = 6000, // broadcast mdaq data
	PKT_S_SEATINGBUCK_HANDLE = 6002, // broadcast inno mfc program

	///////////////////////////////////////////////////////////
	// 언리얼이 보내는 데이터 7000
	///////////////////////////////////////////////////////////
	PKT_C_DRIVING_STATE = 7000, // ue -> server
	PKT_S_DRIVING_STATE = 7001, // server -> ivi

	PKT_C_AUTO_DRIVE = 7002,
	PKT_S_AUTO_DRIVE = 7003,

	PKT_C_DONE_AUTO_DRIVE = 7004,
	PKT_S_DONE_AUTO_DRIVE = 7005,
};

class ClientPacketHandler
{
public:
	static void HandlePacket(std::shared_ptr<PacketData> pkt);

	// 주행 전 데이터 입력
	static void HandleSetInfo(std::shared_ptr<PacketData> pkt);

	// MDAQ -> INNO Seatingbuck Button data
	static void HandleMDAQData(std::shared_ptr<PacketData> pkt);

	// UE -> Server
	static void HandleDrivingState(std::shared_ptr<PacketData> pkt);

	// 자율주행
	static void HandleAutoDrive(std::shared_ptr<PacketData> pkt);
	static void HandleDoneAutoDrive(std::shared_ptr<PacketData> pkt);

	// 전송 전용
	static void Broadcast(PacketHeader& header, std::string& jsonString);
};
