#include "pch.h"
#include "IocpCore.h"
#include "PacketQueue.h"
#include "SendQueue.h"
#include "HttpCore.h"
#include "ClientJsonHandler.h"
#include "DBConnectionPool.h"

/*
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

void TestDB()
{
	const WCHAR* cs = L"Driver={MySQL ODBC 9.1 Unicode Driver};Server=127.0.0.1;Database=testhdi;UID=root;PWD=tomatosoup1!";
	int32 MAX_DB_CONNECTION = 1;
	ASSERT_CRASH(GDBConnectionPool->Connect(MAX_DB_CONNECTION, cs));

	// Write
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		dbConn->Unbind();

		WCHAR insertName[100] = L"KJ";
		SQLLEN insertNameLen = 0;
		int32 insertAge = 30;
		SQLLEN insertAgeLen = 0;
		dbConn->BindParam(1, insertName, &insertNameLen);
		dbConn->BindParam(2, &insertAge, &insertAgeLen);
		/*dbConn->BindParam(1, SQL_C_WCHAR, SQL_WCHAR, sizeof(insertName), &insertName, &insertNameLen);
		dbConn->BindParam(2, SQL_C_LONG, SQL_INTEGER, sizeof(insertAge), &insertAge, &insertAgeLen);*/
		dbConn->Execute(L"INSERT INTO user (name, age) VALUES (?, ?);");
		GDBConnectionPool->Push(dbConn);
	}

	// Read
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		dbConn->Unbind();
		auto query = L" \
		SELECT * FROM user; ";

		int32 id = 0;
		SQLLEN outIdLen = 0;
		dbConn->BindCol(1, SQL_C_SLONG, sizeof(id), &id, &outIdLen);

		WCHAR name[256] = { 0 };
		SQLLEN outNameLen = 0;
		dbConn->BindCol(2, SQL_C_WCHAR, sizeof(name), &name, &outNameLen);

		int32 age = 0;
		SQLLEN outAgeLen = 0;
		dbConn->BindCol(3, SQL_C_SLONG, sizeof(age), &age, &outAgeLen);

		TIMESTAMP_STRUCT dateTime = { 0 };
		SQLLEN outDateTimeLen = 0;
		dbConn->BindCol(4, SQL_C_TYPE_TIMESTAMP, sizeof(dateTime), &dateTime, &outDateTimeLen);

		dbConn->Execute(query);
		while (dbConn->Fetch())
		{
			wcout.imbue(locale("kor"));
			wcout << id << ", " << name << ", " << age << ", "
				<< dateTime.year << "-" << dateTime.month << "-" << dateTime.day
				<< " " << dateTime.hour << ":" << dateTime.minute << ":" << dateTime.second
				<< '\n';
		}

		GDBConnectionPool->Push(dbConn);
	}
}

int main()
{
	/*GDBConnectionPool = new DBConnectionPool;
	TestDB();
	return 0;*/

	///StartHttpServer();

	// TCP Server Set
	int32 MAX_SESSION_COUNT = 1;
	shared_ptr<IocpCore> iocpCore = make_shared<IocpCore>(
		NetAddress(L"192.168.10.123", 7777)
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
	delete GDBConnectionPool;

	return 0;
}

void IocpWorker(shared_ptr<IocpCore> iocpCore)
{
	while (true)
	{
		iocpCore->GQCS(10);
		if (sendQueue->Size() > 0) sendQueue->PopSend();
	}
}

void PacketWorker()
{
	while (true)
	{
		if (pktQueue->Size() > 0) pktQueue->ProcessPacket();
	}
}