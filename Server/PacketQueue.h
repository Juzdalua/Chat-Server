#pragma once
class Session;

struct PacketData
{
	BYTE* buffer;
	int32 len;
	shared_ptr<Session> session;
};

class PacketQueue
{
public:
	PacketQueue() = default;
	~PacketQueue() {};

public:
	void Push(PacketData _pkt);
	void ProcessPacket();
	int32 Size() { return _queue.size(); }

private:
	queue<PacketData> _queue;
	mutex _lock;
};

extern unique_ptr<PacketQueue> pktQueue;
