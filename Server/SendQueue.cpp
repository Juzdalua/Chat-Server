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
	SendData sendData = { 0 };
	{
		std::lock_guard<std::mutex> lock(_lock);
		if (_queue.empty()) return;

		sendData = _queue.front();
		_queue.pop();
	}

	if (sendData.session != nullptr) sendData.session->Send(sendData.sendBuffer);
	else Broadcast(sendData.sendBuffer);

	sendData.sendBuffer = nullptr;
	sendData.session = nullptr;
}

void SendQueue::Broadcast(std::shared_ptr<SendBuffer> sendBuffer)
{
	if (_iocpCore == nullptr)
	{
		//Utils::AlertOK(_T("Broadcast Set Error."), MB_ICONERROR);
		return;
	}
	_iocpCore->Broadcast(sendBuffer);
}
