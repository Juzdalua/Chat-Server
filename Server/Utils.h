#pragma once
#include <cstdlib>
#include <fstream>
#include <string>

class Utils
{
public:
	Utils() = delete;

public:
	static void EnvInit(const std::string& fileName = ".env");
	static std::string getEnv(const std::string& key);

	static map<string, string> GetEnv() { return _envVariables; }

private:
	static map<std::string, std::string> _envVariables;
};