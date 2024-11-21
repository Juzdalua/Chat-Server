#pragma once
class Session;
class SendBuffer;

struct SendData
{
	shared_ptr<SendBuffer> sendBuffer;
	shared_ptr<Session> session;
};

class SendQueue
{
public:
	SendQueue() = default;
	~SendQueue() {};

public:
	void Push(const SendData& sendData);
	void PopSend();
	int32 Size() { return _queue.size(); }

private:
	queue<SendData> _queue;
	mutex _lock;
};

extern unique_ptr<SendQueue> sendQueue;
