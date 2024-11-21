#pragma once
#include "DBConnection.h"

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool Connect(int32 connectionCount, const WCHAR* connectionString);
	void Clear();

	DBConnection* Pop();
	void Push(DBConnection* connection);

private:
	mutex _lock;
	SQLHENV _enviroment = SQL_NULL_HANDLE;
	vector<DBConnection*> _connections;
};

extern class DBConnectionPool* GDBConnectionPool;