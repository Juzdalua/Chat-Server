#include "pch.h"
#include "DBConnectionPool.h"

DBConnectionPool* GDBConnectionPool = nullptr;

DBConnectionPool::DBConnectionPool()
{
}

DBConnectionPool::~DBConnectionPool()
{
	Clear();
}

bool DBConnectionPool::Connect(int32 connectionCount, const WCHAR* connectionString)
{
	lock_guard<mutex> lock(_lock);
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_enviroment) != SQL_SUCCESS) return false;
	if (SQLSetEnvAttr(_enviroment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0)) return false;

	for (int32 i = 0; i < connectionCount; i++)
	{
		DBConnection* connection = new DBConnection;

		if (connection->Connect(_enviroment, connectionString) == false) return false;

		_connections.push_back(connection);
	}
	return true;
}

void DBConnectionPool::Clear()
{
	lock_guard<mutex> lock(_lock);
	if (_enviroment != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, _enviroment);
		_enviroment = SQL_NULL_HANDLE;
	}

	for (DBConnection* connection : _connections)
		delete connection;
	_connections.clear();
}

DBConnection* DBConnectionPool::Pop()
{
	lock_guard<mutex> lock(_lock);
	if(_connections.empty()) return nullptr;

	DBConnection* connection = _connections.back();
	_connections.pop_back();
	return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
	lock_guard<mutex> lock(_lock);
	_connections.push_back(connection);
}
