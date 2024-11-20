#pragma once

class Session;
class SendBuffer;

enum class EventType :uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);
	void Init();

public:
	EventType _eventType;
	shared_ptr<Session> sessionRef;
};

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() :IocpEvent(EventType::Connect) {}
};

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() :IocpEvent(EventType::Disconnect) {}
};

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() :IocpEvent(EventType::Accept) {}
};

class RecvEvent :public IocpEvent
{
public:
	RecvEvent() :IocpEvent(EventType::Recv) {}
};

class SendEvent :public IocpEvent
{
public:
	SendEvent() :IocpEvent(EventType::Send) {}
	vector<shared_ptr<SendBuffer>> sendBuffers;
};