#include "pch.h"
#include "ClientJsonHandler.h"

json SserializeJson(const string& message)
{
	return json::parse(message);
}

// string s = R"({"name": "Alice", "age": 25})";
string DeserializeJson(const json& jsonString)
{
	return jsonString.dump();
}
