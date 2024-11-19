#include "pch.h"
#include "IocpCore.h"
#include "PacketQueue.h"
#include "SendQueue.h"

/*
	API 스레드 1개
	클라이언트 IO 수신 및 송신 스레드 1개
	TCP 메인로직처리 스레드 1개
*/

void IocpWorker(shared_ptr<IocpCore> iocpCore);
void PacketWorker();

int main()
{
	// Server Set
	shared_ptr<IocpCore> iocpCore = make_shared<IocpCore>();
	iocpCore->StartServer();

	// Client Set
	shared_ptr<Session> session = make_shared<Session>();
	iocpCore->StartAccept(session);

	// Thread
	vector<thread> workers;
	workers.emplace_back(IocpWorker, iocpCore);
	workers.emplace_back(PacketWorker);

	// Exit
	for (auto& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}
	return 0;
}

void IocpWorker(shared_ptr<IocpCore> iocpCore)
{
	while (true)
	{
		iocpCore->Dispatch(10);

		//if (sendQueue->Size() > 0) sendQueue->PopSend();
	}
}

void PacketWorker()
{
	while (true)
	{
		if (pktQueue->Size() > 0) pktQueue->ProcessPacket();
	}
}