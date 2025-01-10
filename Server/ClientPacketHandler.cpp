#include "pch.h"
#include "ClientPacketHandler.h"
#include "SendBuffer.h"
#include "SendQueue.h"
#include "GameData.h"
#include "Utils.h"
//#include "DBConnectionPool.h"

void ClientPacketHandler::HandlePacket(std::shared_ptr<PacketData> pkt)
{
	try
	{
		//////////////////////////////////////////////////////////////////
		// 헤더 파싱
		PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt->buffer);
		/*int id = ntohl(recvHheader->id);
		int size = ntohl(recvHheader->size);*/

		int id = recvHheader->id;
		int size = recvHheader->size;
		//////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////
		// 자율주행
		switch (id)
		{
		case PKT_AUTO_DRIVE:
			HandleAutoDrive(pkt);
			return;

		case PKT_DONE_AUTO_DRIVE:
			HandleDoneAutoDrive(pkt);
			return;
		}
		//////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////
		// 수신 후 브로드캐스트
		PacketHeader header = { 0 };
		header.id = id;

		std::string jsonString(reinterpret_cast<char*>(pkt->buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
		json jsonData;

		try {
			jsonData = json::parse(jsonString);
		}
		catch (const json::parse_error& e) {
			Utils::LogError("JSON parsing error: " + std::string(e.what()) + " with string: " + jsonString, "HandlePacket");
			jsonData.clear();
			return;
		}

		json resultJson;
		resultJson["result"] = jsonData;
		jsonString = resultJson.dump();

		Broadcast(header, jsonString);
		jsonData.clear();
		resultJson.clear();
		jsonString.clear();
		return;
		//////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////
		// Deprecated
		switch (id)
		{
		case PKT_SET_INFO:
			HandleSetInfo(pkt);
			return;

		case PKT_SEATINGBUCK_BUTTON:
			HandleMDAQData(pkt);
			return;

		case PKT_DRIVING_STATE:
			HandleDrivingState(pkt);
			return;

		case PKT_AUTO_DRIVE:
			HandleAutoDrive(pkt);
			return;

		case PKT_DONE_AUTO_DRIVE:
			HandleDoneAutoDrive(pkt);
			return;
		}
		//////////////////////////////////////////////////////////////////
	}
	catch (const std::exception& e)
	{
		Utils::LogError(e.what(), "HandlePacket");
	}
}

void ClientPacketHandler::HandleSetInfo(std::shared_ptr<PacketData> pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt->buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_SET_INFO;

	std::string jsonString(reinterpret_cast<char*>(pkt->buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
	json jsonData = json::parse(jsonString);
	gameData->SetMod(static_cast<Mod>(jsonData["drivingMode"]));
	gameData->SetScenarioMap(static_cast<ScenarioMap>(jsonData["simRacingMap"]));
	gameData->SetSimMap(static_cast<SimMap>(jsonData["drivingMap"]));
	gameData->SetWeatherType(static_cast<WeatherType>(jsonData["weather"]));
	gameData->SetTimeType(static_cast<TimeType>(jsonData["time"]));
	gameData->SetTrafficType(static_cast<TrafficType>(jsonData["traffic"]));
	gameData->SetCarType(static_cast<CarType>(jsonData["carSelection"]));

	if (!jsonData.contains("pageNum")) jsonData["pageNum"] = 1;
	json resultJson;
	resultJson["result"] = jsonData;
	jsonString = resultJson.dump();

	//json jsonData = {
	//	{"result", {
	//		{"map", "pkt"},
	//		{"car", "pkt"},
	//		{"destination", "pkt"},
	//		{"destination", {
	//			{"name", "pkt"},
	//			{"posX", "pkt"},
	//			{"posY", "pkt"},
	//			{"posZ", "pkt"},
	//		}},
	//		{"traffic", "pkt"},
	//		{"weather", "pkt"},
	//	}}
	//};
	//std::string jsonString = jsonData.dump(); // JSON -> String
	Broadcast(header, jsonString);
	jsonData.clear();
	resultJson.clear();
	jsonString.clear();
}

void ClientPacketHandler::HandleMDAQData(std::shared_ptr<PacketData> pkt)
{
	//if (!theDlg.IsRun()) return;

	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt->buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_SEATINGBUCK_BUTTON;

	std::string jsonString(reinterpret_cast<char*>(pkt->buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
	json jsonData = json::parse(jsonString);
	json resultJson;
	resultJson["result"] = jsonData;
	jsonString = resultJson.dump();

	Broadcast(header, jsonString);
	jsonData.clear();
	resultJson.clear();
	jsonString.clear();
}

void ClientPacketHandler::HandleDrivingState(std::shared_ptr<PacketData> pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt->buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_DRIVING_STATE;

	std::string jsonString(reinterpret_cast<char*>(pkt->buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
	json jsonData = json::parse(jsonString);
	json resultJson;
	resultJson["result"] = jsonData;
	jsonString = resultJson.dump();

	/*gameData->SetSection(jsonData["section"]);
	gameData->SetStatus(jsonData["status"]);

	if (gameData->GetStatus() == Status::Done)
	{
		gameData->Clear();
	}*/

	Broadcast(header, jsonString);
	jsonData.clear();
	resultJson.clear();
	jsonString.clear();
}

void ClientPacketHandler::HandleAutoDrive(std::shared_ptr<PacketData> pkt)
{
	//theDlg.AutoDrive(pkt);
}

void ClientPacketHandler::HandleDoneAutoDrive(std::shared_ptr<PacketData> pkt)
{
	//theDlg.DoneAutoDrive(pkt);
}

void ClientPacketHandler::Broadcast(PacketHeader& header, std::string& jsonString)
{
	UINT jsonSize = static_cast<UINT>(jsonString.size());
	int totalPacketSize = jsonSize + sizeof(PacketHeader);
	header.size = totalPacketSize;

	std::vector<unsigned char> buffer(totalPacketSize);
	memcpy(buffer.data(), &header, sizeof(PacketHeader));
	memcpy(buffer.data() + sizeof(PacketHeader), jsonString.data(), jsonSize);

	std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(4096);
	/*reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id = id;
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size = size;*/

	sendBuffer->CopyData(buffer.data(), totalPacketSize);
	sendQueue->Push({ sendBuffer, nullptr });
	/*CString testMsg;
	testMsg.Format(_T("id: %d, size: %d / id: %d, size: %d"), id, size, reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id, reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size);
	Utils::AlertOK(testMsg, MB_ICONINFORMATION);*/
}

