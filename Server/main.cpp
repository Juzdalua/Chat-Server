#include "pch.h"
#include "IocpCore.h"
#include "PacketQueue.h"
#include "SendQueue.h"
#include "HttpCore.h"
#include "DBConnectionPool.h"
#include "SendBuffer.h"
#include "ClientPacketHandler.h"

/*
	클라이언트 IO 수신 및 송신 스레드 1개
	TCP 메인로직처리 스레드 1개
*/

wstring _IP = L"192.168.10.123";
int _PORT = 1998;

void IocpWorker(shared_ptr<IocpCore> iocpCore);
void PacketWorker();
void SeatingbuckSendData(shared_ptr<IocpCore> iocpCore);
void SeatingbuckSendData1();
void SeatingbuckSendData2();

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
		NetAddress(_IP, _PORT)
	);
	iocpCore->StartServer();

	// Client Set
	iocpCore->StartAccept();

	sendQueue->SetIocpCore(iocpCore);

	// Thread
	vector<thread> workers;
	workers.emplace_back(IocpWorker, iocpCore);
	workers.emplace_back(PacketWorker);
	workers.emplace_back(SeatingbuckSendData, iocpCore);
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

void SeatingbuckSendData(shared_ptr<IocpCore> iocpCore)
{
	while (true)
	{
		this_thread::sleep_for(1s);
		if (iocpCore->CanBroadcast())
		{
			SeatingbuckSendData1();
			SeatingbuckSendData2();
		}
	}
}

void SeatingbuckSendData1()
{
	struct _stOutputTorque	// ASWS에서 결과값으로 나오는 토크값
	{
		BYTE	bState;		// 상태정보
		BYTE	bMode;		// 모드정보
		float	fPulse;		// 펄스값
		float	fVelocity;	// 각속도
		float	fAngle;		// 핸들각도
		float	fOutTorque;	// 아웃 토크값
		float	fInTorque;	// 인 토크값
	};

	_stOutputTorque data = { 1,0,123.45f,10.5f,30.0f,5.5f,3.2f };

	json jsonData = {
		{"result", {
			{"bState", data.bState},
			{"bMode", data.bMode},
			{"fPulse", data.fPulse},
			{"fVelocity", data.fVelocity},
			{"fAngle", data.fAngle},
			{"fOutTorque", data.fOutTorque},
			{"fInTorque", data.fInTorque},
		}}
	};
	std::string jsonString = jsonData.dump(); // JSON -> String

	// JSON 문자열이 유효한지 파싱해서 확인
	try {
		json::parse(jsonString);
	}
	catch (json::parse_error& e) {
		cout << e.what() << '\n';
		return;
	}

	UINT jsonSize = static_cast<UINT>(jsonString.size());

	// 패킷 헤더 생성
	PacketHeader header = { 0 };
	header.size = sizeof(PacketHeader) + jsonSize;
	header.id = 6000;

	cout << "Header Size -> " << sizeof(PacketHeader) << ", id -> " << header.id << ", size -> " << header.size << '\n';

	// buffer에 데이터 복사
	std::vector<unsigned char> buffer(header.size);
	memcpy(buffer.data(), &header, sizeof(PacketHeader));
	memcpy(buffer.data() + sizeof(PacketHeader), jsonString.data(), jsonSize);

	std::shared_ptr<SendBuffer> sendBuffer = std::shared_ptr<SendBuffer>(new SendBuffer(4096));
	sendBuffer->CopyData(buffer.data(), header.size);
	sendQueue->Push({ sendBuffer, nullptr });
}
void SeatingbuckSendData2()
{
	struct _DIGITAL_IN_16 {
		unsigned short bMF1 : 1;
		unsigned short bMF2 : 1;
		unsigned short bMF3 : 1;
		unsigned short bMF4 : 1;
		unsigned short bMF5 : 1;
		unsigned short bSideBrake : 1;
		unsigned short bSeatBelt : 1;
		unsigned short bBrakeLamp : 1;
		unsigned short bHorn : 1;
		unsigned short bODOff : 1;
		unsigned short bBEAM_Panel : 1;
		unsigned short bBEAM_Low : 1;
		unsigned short bBEAM_High : 1;
		unsigned short Wiper_Slow : 1;
		unsigned short Wiper_Fast : 1;
		unsigned short Wiper_Washer : 1;
	};

	struct _DIGITAL_IN_5 {
		unsigned short DigitalReserved1 : 1;
		unsigned short DigitalReserved2 : 1;
		unsigned short DigitalReserved3 : 1;
		unsigned short DigitalReserved4 : 1;
		unsigned short DigitalReserved5 : 1;
	};

	// MDAQ -> Model.
	struct _CabinData {
		double	Gear;				// 0, 1, 2, 4, 8, 16, 32
		double	Handle;				// -540 ~ 540(Left ~ Right)
		double	Throttle;			// 0 ~ 255
		double	Brake;				// 0~255
		double	ignitionKey;		// // 0:OFF, 1:ACC, 2:ON, 3:START
		double	indicator;  		// 0: Lamp Off, 1: Right,  2: Left 3:Waring
		double  dbClutch;			// Clutch
		float	fTorque;			// TRW Torque Value;
		BYTE	BrkLamp;			// 0,1(?)
		BYTE	byExhaustBrake;	    //배기브레이크 0 1
		BYTE byWheeltype;//2: 2륜구동,  4:4륜구동
		BYTE	byDokDoLamp;	//  0 or 1.0
		BYTE byTransmission; // 중간변속기1:보통, 2:저속, 3:중립
		BYTE byEmergency_LampSwitch; //선택스위치 1:평상시, 2:비상1단, 3:비상2단, 4:오프
		BYTE byPTO;  //동력인출  작동 : 1, 차단:0
		float fAnalogReserved1;
		float fAnalogReserved2;
		float fAnalogReserved3;
		float fAnalogReserved4;
		float fAnalogReserved5;
		_DIGITAL_IN_16  Din;
		_DIGITAL_IN_5 DInReserved;
	};

	_DIGITAL_IN_16 din = { 1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1 };
	_DIGITAL_IN_5 dInReserved = { 10,20,30,40,50 };

	_CabinData stOldData = { 3,1.5f,0.8f,0.2f,true,1,0.9f,250.5f,1,0,2,0,1,0,1,0.0f,1.5f,2.7f,3.3f,4.1f, din, dInReserved };

	json jsonData = {
		{"result", {
			{"Gear", stOldData.Gear},
			{"Handle", stOldData.Handle},
			{"Throttle", stOldData.Throttle},
			{"Brake", stOldData.Brake},
			{"ignitionKey", stOldData.ignitionKey},
			{"indicator", stOldData.indicator},
			{"dbClutch", stOldData.dbClutch},
			{"fTorque", stOldData.fTorque},
			{"BrkLamp", stOldData.BrkLamp},
			{"byExhaustBrake", stOldData.byExhaustBrake},
			{"byWheeltype", stOldData.byWheeltype},
			{"byDokDoLamp", stOldData.byDokDoLamp},
			{"byTransmission", stOldData.byTransmission},
			{"byEmergency_LampSwitch", stOldData.byEmergency_LampSwitch},
			{"byPTO", stOldData.byPTO},
			{"fAnalogReserved1", stOldData.fAnalogReserved1},
			{"fAnalogReserved2", stOldData.fAnalogReserved2},
			{"fAnalogReserved3", stOldData.fAnalogReserved3},
			{"fAnalogReserved4", stOldData.fAnalogReserved4},
			{"fAnalogReserved5", stOldData.fAnalogReserved5},
			{"Din", {
				{"bMF1", static_cast<int>(stOldData.Din.bMF1)},
				{"bMF2", static_cast<int>(stOldData.Din.bMF2)},
				{"bMF3", static_cast<int>(stOldData.Din.bMF3)},
				{"bMF4", static_cast<int>(stOldData.Din.bMF4)},
				{"bMF5", static_cast<int>(stOldData.Din.bMF5)},
				{"bSideBrake", static_cast<int>(stOldData.Din.bSideBrake)},
				{"bSeatBelt", static_cast<int>(stOldData.Din.bSeatBelt)},
				{"bBrakeLamp", static_cast<int>(stOldData.Din.bBrakeLamp)},
				{"bHorn", static_cast<int>(stOldData.Din.bHorn)},
				{"bODOff", static_cast<int>(stOldData.Din.bODOff)},
				{"bBEAM_Panel", static_cast<int>(stOldData.Din.bBEAM_Panel)},
				{"bBEAM_Low", static_cast<int>(stOldData.Din.bBEAM_Low)},
				{"bBEAM_High", static_cast<int>(stOldData.Din.bBEAM_High)},
				{"Wiper_Slow", static_cast<int>(stOldData.Din.Wiper_Slow)},
				{"Wiper_Fast", static_cast<int>(stOldData.Din.Wiper_Fast)},
				{"Wiper_Washer", static_cast<int>(stOldData.Din.Wiper_Washer)}
			}},
			{"DInReserved", {
				{"DigitalReserved1", static_cast<int>(stOldData.DInReserved.DigitalReserved1)},
				{"DigitalReserved2", static_cast<int>(stOldData.DInReserved.DigitalReserved2)},
				{"DigitalReserved3", static_cast<int>(stOldData.DInReserved.DigitalReserved3)},
				{"DigitalReserved4", static_cast<int>(stOldData.DInReserved.DigitalReserved4)},
				{"DigitalReserved5", static_cast<int>(stOldData.DInReserved.DigitalReserved5)}
			}}
		}}
	};

	std::string jsonString = jsonData.dump(); // JSON -> String
	try {
		json::parse(jsonString); // JSON 파싱 시도
	}
	catch (json::parse_error& e) {
		cout << e.what() << '\n';
		return;
	}

	UINT jsonSize = static_cast<UINT>(jsonString.size());
	int totalPacketSize = jsonSize + sizeof(PacketHeader);

	PacketHeader header = { 0 };
	header.size = sizeof(PacketHeader) + jsonSize;
	header.id = 6001;

	cout << "Header Size -> " << sizeof(PacketHeader) << ", id -> " << header.id << ", size -> " << header.size << '\n';

	std::vector<unsigned char> buffer(totalPacketSize);
	memcpy(buffer.data(), &header, sizeof(PacketHeader));
	memcpy(buffer.data() + sizeof(PacketHeader), jsonString.data(), jsonSize);

	std::shared_ptr<SendBuffer> sendBuffer = std::shared_ptr<SendBuffer>(new SendBuffer(4096));
	sendBuffer->CopyData(buffer.data(), totalPacketSize);
	sendQueue->Push({ sendBuffer, nullptr });
}