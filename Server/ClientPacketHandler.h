#pragma once
#include <json.hpp>
#include <string>
#include "PacketQueue.h"
#include "Session.h"

using json = nlohmann::json;

enum : UINT
{
	PKT_C_SET_INFO = 5000,
	PKT_S_SET_INFO = 5001,

	PKT_S_SEATINGBUCK_HANDLE = 6000, // broadcast this mfc program

	PKT_C_SEATINGBUCK_BUTTON = 6001, // recv from mdaq
	PKT_S_SEATINGBUCK_BUTTON = 6002, // broadcast mdaq data

	PKT_C_DRIVING_STATE = 6003,
	PKT_S_DRIVING_STATE = 6004,
};

class ClientPacketHandler
{
public:
	static void HandlePacket(PacketData& pkt);

	// 주행 전 데이터 입력
	static void HandleSetInfo(PacketData& pkt);

	// MDAQ -> INNO Seatingbuck Button data
	static void HandleMDAQData(PacketData& pkt);

	// UE -> Server
	static void HandleDrivingState(PacketData& pkt);

	// 전송 전용
	static void Broadcast(PacketHeader& header, std::string& jsonString);
};

json SserializeJson(const std::string& message);
std::string DeserializeJson(const json& jsonString);