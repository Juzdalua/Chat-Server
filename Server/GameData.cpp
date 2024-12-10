#include "pch.h"
#include "GameData.h"

//shared_ptr<GameData> gameData;
GameData* gameData = nullptr;

GameData::GameData()
{
}

GameData::~GameData()
{
	Clear();
	Destroy();
}

bool GameData::CanStart()
{
	if (_mod == Mod::Empty) return false;
	if (_scenarioMap == ScenarioMap::Empty && _simMap == SimMap::Empty) return false;
	if (_weatherType == WeatherType::Empty) return false;
	if (_timeType == TimeType::Empty) return false;
	if (_trafficType == TrafficType::Empty) return false;
	if (_carType == CarType::Empty) return false;

	return true;
}

void GameData::Clear()
{
	_mod = Mod::Empty;
	_scenarioMap = ScenarioMap::Empty;
	_simMap = SimMap::Empty;
	_weatherType = WeatherType::Empty;
	_timeType = TimeType::Empty;
	_trafficType = TrafficType::Empty;
	_carType = CarType::Empty;
}

void GameData::Destroy()
{
}
