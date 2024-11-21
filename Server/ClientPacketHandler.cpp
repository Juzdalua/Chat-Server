#include "pch.h"
#include "ClientPacketHandler.h"
#include "SendBuffer.h"
#include "SendQueue.h"

bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, shared_ptr<Session>& session)
{
	cout << "Handle Packet = " << buffer << '\n';
	shared_ptr<SendBuffer> sendBuffer = shared_ptr<SendBuffer>(new SendBuffer(4096));
	sendBuffer->CopyData(buffer, len);
	//session->Send(sendBuffer);
	sendQueue->Push({ sendBuffer, session });

	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	switch (recvHeader->id)
	{
	case PKT_C_TEST:
		ClientPacketHandler::HandleTest(buffer, len, session);
		break;
	}

	return false;
}

bool ClientPacketHandler::HandleTest(BYTE* buffer, int32 len, shared_ptr<Session>& session)
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
