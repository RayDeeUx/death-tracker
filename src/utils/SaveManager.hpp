#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;
typedef std::vector<int> Deaths;
typedef std::map<int, bool> Progresses;

class SaveManager {
private:
	static GJGameLevel* m_level;
	static int m_levelCount;
	static Deaths m_deaths;
	static Deaths m_sessionDeaths;
	static Progresses m_progresses;
	static int m_checkpoint;
	static bool m_usingStartpos;
	static bool m_shouldResetSessionDeaths;

	static void createBackup();
	static std::string getLevelId();
	static void calcDeathsAndProgresses();

public:
	SaveManager() = delete;

	static GJGameLevel* getLevel();
	static void setLevel(GJGameLevel* level);
	static bool isPlatformer();
	static bool isNewBest(int percent);
	static void incLevelCount();
	static void setShouldResetSessionDeaths();
	static void resetSessionDeaths();
	static Deaths getDeaths();
	static bool hasNoDeaths(bool checkSessionDeaths = false);
	static void addDeath(int percent = 0);
	static void allocateDeathsForCheckpoint();
	static void incCheckpoint();
	static void resetCheckpoint();
	static bool isUsingStartpos();
	static void setUsingStartpos(bool usingStartpos);
};