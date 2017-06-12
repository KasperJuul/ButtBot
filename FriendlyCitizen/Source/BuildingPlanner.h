#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "InformationManager.h"
enum TypeDec{UnitDec,TechDec,UpgradeDec};

struct Priority{
	//ENUM TYPE
	TypeDec declaration;
	int priority = INT_MAX;
	BWAPI::UnitType unitType;
	BWAPI::TechType techType;
	BWAPI::UpgradeType upgradeType;
	//std::string typeName = ""; //The name of the thing to be built or researched

	bool operator < (const Priority& str) const
	{
		return (priority < str.priority);
	}
	bool operator > (const Priority& str) const
	{
		return (priority > str.priority);
	}
};

class BuildingPlanner{
public:
	static BWAPI::UnitType chooseBetweenMilitary(std::vector<TechNode> selection);
	static std::vector<float> techValue(std::vector<int> possibleNodes, std::vector<int> futureNodes, std::vector<float> totalValue);
	static std::vector<Priority> order(std::vector<Priority> military, std::vector<Priority> economy, Priority technology);
	static std::vector<Priority> findOrder();
	static float specialValue(TechNode toAnalyze);//Depcrecated
	static float specialValue(UnitType toAnalyze);
	static float econValue(TechNode toAnalyze);//Depcrecated
	static float econValue(UnitType toAnalyze);
	static float combatValue(TechNode toAnalyze);//Depcrecated
	static float combatValue(UnitType toAnalyze);
	static BWAPI::UnitType makePlan();
	static std::vector<Priority> makePlanN();
	static void plannerOnFrame();
	static int heuristic(TechNode node);
	static std::string plan;
};