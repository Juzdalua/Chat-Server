#pragma once

enum class Mod : UINT
{
	Empty,
	Scenario,
	Sim,
	Scanner,
	Campaign
};

enum class ScenarioMap : UINT
{
	Empty,
	Seoul,
	SanFrancisco,
};

enum class SimMap : UINT
{
	Empty,
	AssettoCorsa,
	Nurburgring,
	INJE,
	BugakSkyway,
};

enum class WeatherType : UINT
{
	Empty,
	Sunny,
	Rain,
};

enum class TimeType : UINT
{
	Empty,
	Noon,
	Night,
};

enum class TrafficType : UINT
{
	Empty,
	Low,
	Mid,
	High,
};

enum class CarType : UINT
{
	Empty,
	Ioiniq5N,
	AvanteN,
};

enum class Section : UINT
{
	Empty,
	Start,
	HDA4Enter,
	ALC,
	AEB,
	HDA4Out,
	RSPA,
};

enum class Status : UINT
{
	Empty,
	Ready,
	Run,
	Done,
};


class GameData
{
public:
	GameData();
	~GameData();

public:
	Mod GetMod() { return _mod; }
	ScenarioMap GetScenarioMap() { return _scenarioMap; }
	SimMap GetSimMap() { return _simMap; }
	WeatherType GetWeatherType() { return _weatherType; }
	TimeType GetTimeType() { return _timeType; }
	TrafficType GetTrafficType() { return _trafficType; }
	CarType GetCarType() { return _carType; }
	Section GetSection() { return _section; }
	Status GetStatus() { return _status; }

	void SetMod(Mod mod) { _mod = mod; }
	void SetScenarioMap(ScenarioMap scenarioMap) { _scenarioMap = scenarioMap; }
	void SetSimMap(SimMap simMap) { _simMap = simMap; }
	void SetWeatherType(WeatherType weatherType) { _weatherType = weatherType; }
	void SetTimeType(TimeType timeType) { _timeType = timeType; }
	void SetTrafficType(TrafficType trafficType) { _trafficType = trafficType; }
	void SetCarType(CarType carType) { _carType = carType; }
	void SetSection(Section section) { _section = section; }
	void SetStatus(Status status) { _status = status; }

	bool CanStart();
	void Clear();
	void Destroy();

private:
	Mod _mod = Mod::Empty;
	ScenarioMap _scenarioMap = ScenarioMap::Empty;
	SimMap _simMap = SimMap::Empty;
	WeatherType _weatherType = WeatherType::Empty;
	TimeType _timeType = TimeType::Empty;
	TrafficType _trafficType = TrafficType::Empty;
	CarType _carType = CarType::Empty;

	Section _section = Section::Empty;
	Status _status = Status::Empty;
};

//extern std::shared_ptr<GameData> gameData;
extern GameData* gameData;