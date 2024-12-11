#include "pch.h"
#include "ClientPacketHandler.h"
#include "SendBuffer.h"
#include "SendQueue.h"
#include "GameData.h"

void ClientPacketHandler::HandlePacket(PacketData& pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt.buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;
	/*cout << "RECV 1:: id -> " << id << ", size -> " << size << '\n';
	if (id > 10'000 || id < 0)
	{
		id = ntohl(recvHheader->id);
		size = ntohl(recvHheader->size);
	}*/

	cout << "RECV:: id -> " << id << ", size -> " << size << '\n';

	switch (id)
	{
	case PKT_C_SET_INFO:
		HandleSetInfo(pkt);
		break;

	case PKT_C_SEATINGBUCK_BUTTON:
		HandleMDAQData(pkt);
		break;

	case PKT_C_DRIVING_STATE:
		HandleDrivingState(pkt);
		break;
	}
}

void ClientPacketHandler::HandleSetInfo(PacketData& pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt.buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_S_SET_INFO;

	std::string jsonString(reinterpret_cast<char*>(pkt.buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
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
}

void ClientPacketHandler::HandleMDAQData(PacketData& pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt.buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_S_SEATINGBUCK_BUTTON;

	std::string jsonString(reinterpret_cast<char*>(pkt.buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));

	Broadcast(header, jsonString);
}

void ClientPacketHandler::HandleDrivingState(PacketData& pkt)
{
	PacketHeader* recvHheader = reinterpret_cast<PacketHeader*>(pkt.buffer);
	int id = recvHheader->id;
	int size = recvHheader->size;

	PacketHeader header = { 0 };
	header.id = PKT_S_DRIVING_STATE;

	std::string jsonString(reinterpret_cast<char*>(pkt.buffer + sizeof(PacketHeader)), size - sizeof(PacketHeader));
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
}

void ClientPacketHandler::Broadcast(PacketHeader& header, std::string& jsonString)
{
	UINT jsonSize = static_cast<UINT>(jsonString.size());
	int totalPacketSize = jsonSize + sizeof(PacketHeader);
	header.size = totalPacketSize;

	std::vector<unsigned char> buffer(totalPacketSize);
	memcpy(buffer.data(), &header, sizeof(PacketHeader));
	memcpy(buffer.data() + sizeof(PacketHeader), jsonString.data(), jsonSize);

	std::shared_ptr<SendBuffer> sendBuffer = std::shared_ptr<SendBuffer>(new SendBuffer(4096));
	/*reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id = id;
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size = size;*/

	sendBuffer->CopyData(buffer.data(), totalPacketSize);
	sendQueue->Push({ sendBuffer, nullptr });

	/*CString testMsg;
	testMsg.Format(_T("id: %d, size: %d / id: %d, size: %d"), id, size, reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id, reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size);
	Utils::AlertOK(testMsg, MB_ICONINFORMATION);*/
}

/*----------------------------------------------------------
	JSON Function
----------------------------------------------------------*/
json SserializeJson(const std::string& message)
{
	return json::parse(message);
}

// string s = R"({"name": "Alice", "age": 25})";
std::string DeserializeJson(const json& jsonString)
{
	return jsonString.dump();
}

void TestJSON()
{
	json j = {
		{"name", "John"},
		{"age", 30},
		{"city", "New York"}
	};
	//cout << DeserializeJson(j) << endl; // JSON -> string

	// JSON 역직렬화
	std::string s = R"({"name": "Alice", "age": 25})";
	json j2 = SserializeJson(s); // string -> JSON
	//cout << "Name: " << j2["name"] << ", Age: " << j2["age"] << endl;
}