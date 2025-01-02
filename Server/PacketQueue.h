#pragma once

class Session;

struct PacketData
{
	BYTE* buffer;
	int len;
	std::shared_ptr<Session> session;
};

class PacketQueue
{
public:
	PacketQueue() = default;
	~PacketQueue() {};

public:
	//void Push(const PacketData& _pkt);
	void Push(const std::shared_ptr<PacketData> _pkt);
	void ProcessPacket();
	int Size() { return _queue.size(); }

private:
	//std::queue<PacketData> _queue;
	std::queue<std::shared_ptr<PacketData>> _queue;
	std::mutex _lock;
};

extern std::unique_ptr<PacketQueue> pktQueue;
