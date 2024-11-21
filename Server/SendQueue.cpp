#include "pch.h"
#include "SendQueue.h"
#include "Session.h"

unique_ptr<SendQueue> sendQueue = make_unique<SendQueue>();

void SendQueue::Push(const SendData& sendData)
{
	lock_guard<mutex> lock(_lock);
	_queue.push(sendData);
}

void SendQueue::PopSend()
{
	SendData sendData;
	{
		lock_guard<mutex> lock(_lock);
		if (_queue.empty()) return;

		sendData = _queue.front();
		_queue.pop();
	}

	sendData.session->Send(sendData.sendBuffer);
}
