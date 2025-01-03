#include "pch.h"
#include "Utils.h"

map<std::string, std::string> Utils::envVariables;

void Utils::Init(const string& filePath)
{
	std::ifstream envFile(filePath);
	ASSERT_CRASH(envFile.is_open());

	std::string line;
	while (std::getline(envFile, line))
	{
		if (line.empty() || line[0] == '#') continue;

		size_t delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			envVariables[key] = value;
		}
	}

	envFile.close();
}

string Utils::getEnv(const string& key)
{
	auto it = envVariables.find(key);
	if (it != envVariables.end()) return it->second;
	return "";
}
