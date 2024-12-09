#pragma once
#include <memory>
#include <queue>
#include <mutex>
#include "IocpCore.h"

class Session;
class SendBuffer;

struct SendData
{
	std::shared_ptr<SendBuffer> sendBuffer;
	std::shared_ptr<Session> session;
};

class SendQueue
{
public:
	SendQueue() = default;
	~SendQueue() {};

public:
	int Size() { return _queue.size(); }
	void SetIocpCore(std::shared_ptr<IocpCore> iocpCore) { _iocpCore = iocpCore; }

	void Push(const SendData& sendData);
	void PopSend();
	void Broadcast(std::shared_ptr<SendBuffer> sendBuffer);

	std::shared_ptr<IocpCore> _iocpCore;
private:
	std::queue<SendData> _queue;
	std::mutex _lock;
};

extern std::unique_ptr<SendQueue> sendQueue;
