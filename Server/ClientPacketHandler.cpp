#include "pch.h"
#include "ClientPacketHandler.h"

bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, shared_ptr<Session> session)
{
	cout << "Handle Packet" << '\n';
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	switch (recvHeader->id)
	{
	case PKT_C_TEST:
		ClientPacketHandler::HandleTest(buffer, len, session);
		break;
	}

	return false;
}

bool ClientPacketHandler::HandleTest(BYTE* buffer, int32 len, shared_ptr<Session> session)
{
	cout << "Handle Test" << '\n';
	/*Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	uint16 packetId = PKT_S_PING;
	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
	GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(pkt, packetId)));*/

	return true;
}
