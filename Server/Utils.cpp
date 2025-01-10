#include "pch.h"
#include "Utils.h"
#include <iomanip>
#include <ctime>
#include <sstream>

std::map<std::string, std::string> Utils::_envVariables;

bool Utils::EnvInit(const std::string& fileName)
{
	TCHAR szCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDir);

	// TCHAR -> std::wstring ��ȯ
	std::wstring wideCurrentDir(szCurrentDir);
	std::string currentDir(wideCurrentDir.begin(), wideCurrentDir.end());

	// ���� ���� ���͸��� ���� ��ġ�� �ִ� .env ���� ��� ����
	std::string fullFilePath = currentDir + "\\" + fileName;

	std::ifstream envFile(fullFilePath);
	if (!envFile.is_open()) return false;

	std::string line;
	while (std::getline(envFile, line))
	{
		if (line.empty() || line[0] == '#') continue;

		size_t delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			_envVariables[key] = value;
		}
	}

	envFile.close();
	return true;
}

std::string Utils::getEnv(const std::string& key)
{
	auto it = _envVariables.find(key);
	if (it != _envVariables.end()) return it->second;
	return "";
}

void Utils::LogError(const std::string& errorMsg, const std::string& functionName, std::string fileName)
{
	TCHAR szCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDir);
	std::wstring wideCurrentDir(szCurrentDir);
	std::string currentDir(wideCurrentDir.begin(), wideCurrentDir.end());

	auto t = std::time(nullptr);
	std::tm tm = {};
	localtime_s(&tm, &t);
	std::ostringstream dateStream;
	dateStream << std::put_time(&tm, "%Y_%m_%d");

	std::string fullFilePath = currentDir + "\\log\\" + fileName + dateStream.str() + ".txt";
	std::ofstream logFile(fullFilePath, std::ios::app);
	if (!logFile.is_open()) return;

	std::ostringstream timeStream;
	timeStream << std::put_time(&tm, "%H:%M:%S");

	logFile << std::endl;
	logFile << "[" << timeStream.str() << "] -> [" << functionName << "]" << std::endl;
	logFile << errorMsg << std::endl;
	logFile.close();
}

void Utils::TestLogError()
{
	try
	{
		throw std::runtime_error("Test Error Log");
	}
	catch (const std::exception& e)
	{
		Utils::LogError(e.what(), "TestLogError");
	}
}
