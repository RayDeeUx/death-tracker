#pragma once

#include <Geode/Geode.hpp>
#include <matjson/stl_serialize.hpp>

using namespace geode::prelude;

typedef std::map<std::string, int> Deaths;
typedef std::map<std::string, int> Runs;
typedef std::set<int> NewBests;

typedef struct {
    int start;
    int end;
} Run;

typedef struct {
    long long lastPlayed;
    Deaths deaths;
    Runs runs;
    NewBests newBests;
    int currentBest;
} Session;

typedef struct {
    Deaths deaths;
    Runs runs;
    NewBests newBests;
    int currentBest;
    std::vector<Session> sessions;
    std::vector<int> RunsToSave;
    std::vector<std::string> LinkedLevels;
    std::string levelName;
    int attempts;
    int difficulty;
} LevelStats;

// matjson fuckery
template <>
struct matjson::Serialize<Session> {
    static Session from_json(const matjson::Value& value) {
        return Session {
            .lastPlayed = value["lastPlayed"].as<long long>(),
            .deaths = value["deaths"].as<Deaths>(),
            .runs = value["runs"].as<Runs>(),
            .newBests = value["newBests"].as<NewBests>(),
            .currentBest = value["currentBest"].as_int(),
        };
    }

    static matjson::Value to_json(const Session& value) {
        auto obj = matjson::Object();
        obj["lastPlayed"] = value.lastPlayed;
        obj["deaths"] = value.deaths;
        obj["runs"] = value.runs;
        obj["newBests"] = value.newBests;
        obj["currentBest"] = value.currentBest;
        return obj;
    }
};

template <>
struct matjson::Serialize<LevelStats> {
    static LevelStats from_json(const matjson::Value& value) {

        LevelStats stats{
            .deaths = value["deaths"].as<Deaths>(),
            .runs = value["runs"].as<Runs>(),
            .newBests = value["newBests"].as<NewBests>(),
            .currentBest = value["currentBest"].as_int(),
            .sessions = value["sessions"].as<std::vector<Session>>()
        };

        if (value.contains("RunsToSave"))
            stats.RunsToSave = value["RunsToSave"].as<std::vector<int>>();

        if (value.contains("LinkedLevels"))
            stats.LinkedLevels = value["LinkedLevels"].as<std::vector<std::string>>();
        
        if (value.contains("levelName"))
            stats.levelName = value["levelName"].as_string();
        else
            stats.levelName = "-1";

        if (value.contains("attempts"))
            stats.attempts = value["attempts"].as_int();
        else
            stats.attempts = -1;

        if (value.contains("difficulty"))
            stats.difficulty = value["difficulty"].as_int();
        else
            stats.difficulty = 0;

        return stats;
    }

    static matjson::Value to_json(const LevelStats& value) {
        auto obj = matjson::Object();
        obj["deaths"] = value.deaths;
        obj["runs"] = value.runs;
        obj["newBests"] = value.newBests;
        obj["currentBest"] = value.currentBest;
        obj["sessions"] = value.sessions;
        obj["RunsToSave"] = value.RunsToSave;
        obj["LinkedLevels"] = value.LinkedLevels;
        obj["levelName"] = value.levelName;
        obj["attempts"] = value.attempts;
        obj["difficulty"] = value.difficulty;
        return obj;
    }
};

class StatsManager {
private:
    static GJGameLevel* m_level;
    static std::set<std::string> m_playedLevels;

    static LevelStats m_levelStats;
    
    static bool m_scheduleCreateNewSession;
    static ghc::filesystem::path m_savesFolderPath;

    // internal functions
    static void saveData();
    static LevelStats loadData(GJGameLevel* level);
    static std::tuple<NewBests, int> calcNewBests(GJGameLevel* level);
    static std::vector<std::string> m_AllFontsMap;

public:
    StatsManager() = delete;

    // main functions
    static void loadLevelStats(GJGameLevel* level);
    static LevelStats getLevelStats(GJGameLevel* level);
    static LevelStats getLevelStats(ghc::filesystem::path level);
    static LevelStats getLevelStats(std::string levelKey);
    static void logDeath(int percent);
    static void logRun(Run run);

    // utility functions
    static long long getNowSeconds();
    static std::string getLevelKey(GJGameLevel* level = m_level);
    static Run splitRunKey(std::string runKey);
    static Session* getSession();
    static void updateSessionLastPlayed(bool save = false);
    static void scheduleCreateNewSession(bool scheduled);
    static bool hasPlayedLevel();
    static std::string toPercentKey(int percent);
    static ghc::filesystem::path getLevelSaveFilePath(GJGameLevel* level = m_level);
    static bool ContainsAtIndex(int startIndex, std::string check, std::string str);
    static ccColor3B inverseColor(ccColor3B color);
    static std::string getFont(int fontID);
    static std::string getFontName(int fontID);
    static std::vector<std::string> getAllFont();
    static void saveData(LevelStats stats, GJGameLevel* level);
    static void saveData(LevelStats stats, std::string levelKey);
    static std::vector<std::pair<std::string, LevelStats>> getAllLevels();
    static std::pair<std::string, std::string> splitLevelKey(std::string levelKey);
    /*
    -1 = auto, 0 = NA, 1 = Easy, 2 = Normal
    3 = Hard, 4 = Harder, 5 = Insane, 6 = Hard Demon
    7 = Easy Demon, 8 = Medium Demon, 9 = Insane Demon, 10 = Extreme Demon
    */
    static int getDifficulty(GJGameLevel* level);
};