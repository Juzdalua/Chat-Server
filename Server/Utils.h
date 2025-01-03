#pragma once
#include <cstdlib>
#include <fstream>
#include <string>

class Utils
{
public:
	Utils() = delete;

public:
	static void Init(const std::string& fileName = ".env");
	static std::string getEnv(const std::string& key);

private:
	static map<std::string, std::string> envVariables;
};