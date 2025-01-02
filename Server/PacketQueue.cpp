#include "pch.h"
#include "PacketQueue.h"
#include "ClientPacketHandler.h"
#include "Session.h"

std::unique_ptr<PacketQueue> pktQueue = std::make_unique<PacketQueue>();

//void PacketQueue::Push(const PacketData& _pkt)
//{
//	std::lock_guard<std::mutex> lock(_lock);
//	_queue.push(_pkt);
//}

void PacketQueue::Push(const std::shared_ptr<PacketData> _pkt)
{
	std::lock_guard<std::mutex> lock(_lock);
	_queue.push(_pkt);
}

void PacketQueue::ProcessPacket()
{
	//PacketData pkt = { 0 };
	std::shared_ptr<PacketData> pkt = { 0 };

	{
		std::lock_guard<std::mutex> lock(_lock);
		if (_queue.empty()) return;

		pkt = _queue.front();
		_queue.pop();
	}

	if (pkt->len == 0)
	{
		pkt->buffer = nullptr;
		return;
	}
	if (pkt->len < sizeof(PacketHeader))
	{
		pkt->buffer = nullptr;
		pkt->len = 0;
		//Utils::AlertOK(_T("Invalid packet length"), MB_ICONERROR);
		return;
	}

	ClientPacketHandler::HandlePacket(pkt);
	pkt->buffer = nullptr;
	pkt->len = 0;
}
