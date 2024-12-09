#include "pch.h"
#include "SendQueue.h"
#include "Session.h"

std::unique_ptr<SendQueue> sendQueue = std::make_unique<SendQueue>();

void SendQueue::Push(const SendData& sendData)
{
	std::lock_guard<std::mutex> lock(_lock);
	_queue.push(sendData);
}

void SendQueue::PopSend()
{
	SendData sendData;
	{
		std::lock_guard<std::mutex> lock(_lock);
		if (_queue.empty()) return;

		sendData = _queue.front();
		_queue.pop();
	}

	if (sendData.session != nullptr)
		sendData.session->Send(sendData.sendBuffer);
	Broadcast(sendData.sendBuffer);
}

void SendQueue::Broadcast(std::shared_ptr<SendBuffer> sendBuffer)
{
	if (_iocpCore == nullptr)
	{
		cout << "Broadcast Set Error." << '\n';
		return;
	}
	_iocpCore->Broadcast(sendBuffer);
}
