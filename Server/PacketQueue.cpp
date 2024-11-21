#include "pch.h"
#include "PacketQueue.h"
#include "ClientPacketHandler.h"

unique_ptr<PacketQueue> pktQueue = make_unique<PacketQueue>();

void PacketQueue::Push(const PacketData& _pkt)
{
	lock_guard<mutex> lock(_lock);
	_queue.push(_pkt);
}

void PacketQueue::ProcessPacket()
{
	PacketData pkt;
	{
		lock_guard<mutex> lock(_lock);
		if (_queue.empty()) return;

		pkt = _queue.front();
		_queue.pop();
	}

	ClientPacketHandler::HandlePacket(pkt.buffer, pkt.len, pkt.session);
}
