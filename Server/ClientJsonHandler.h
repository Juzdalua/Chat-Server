#pragma once
#include <json.hpp>

using json = nlohmann::json;

class ClientJsonHandler
{
};

json SserializeJson(const string& message);
string DeserializeJson(const json& jsonString);