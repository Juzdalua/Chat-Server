#pragma once
#include "Session.h"

enum : uint16
{
	PKT_C_TEST = 1000,
	PKT_S_TEST = 1001,
};

class ClientPacketHandler
{
public:
	static bool HandlePacket(BYTE* buffer, int32 len, shared_ptr<Session> session);

	static bool HandleTest(BYTE* buffer, int32 len, shared_ptr<Session> session);
};

