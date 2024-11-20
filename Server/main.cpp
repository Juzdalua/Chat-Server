#include "pch.h"
#include "IocpCore.h"
#include "PacketQueue.h"
#include "SendQueue.h"
#include "HttpCore.h"
#include "ClientJsonHandler.h"

/*
	API 스레드 1개
	클라이언트 IO 수신 및 송신 스레드 1개
	TCP 메인로직처리 스레드 1개
*/

void IocpWorker(shared_ptr<IocpCore> iocpCore);
void PacketWorker();

void TestJSON()
{
	json j = {
		{"name", "John"},
		{"age", 30},
		{"city", "New York"}
	};
	cout << DeserializeJson(j) << endl; // JSON -> string

	// JSON 역직렬화
	string s = R"({"name": "Alice", "age": 25})";
	json j2 = SserializeJson(s); // string -> JSON
	cout << "Name: " << j2["name"] << ", Age: " << j2["age"] << endl;
}

void StartHttpServer()
{
	try {
		net::io_context ioc;
		tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));

		cout << "Server started on port 8080" << endl;

		while (true) {
			tcp::socket socket(ioc);
			acceptor.accept(socket);

			handle_request(socket);
		}
	}
	catch (exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
}


int main()
{
	// TCP Server Set
	int32 MAX_SESSION_COUNT = 1;
	shared_ptr<IocpCore> iocpCore = make_shared<IocpCore>(
		NetAddress(L"127.0.0.1", 7777)
	);
	iocpCore->StartServer();

	// Client Set
	iocpCore->StartAccept();

	// Thread
	vector<thread> workers;
	workers.emplace_back(IocpWorker, iocpCore);
	workers.emplace_back(PacketWorker);
	//workers.emplace_back(StartHttpServer);

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