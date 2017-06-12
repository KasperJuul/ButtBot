#pragma once
#include "InformationManager.h"
#include "CostumUnit.h"
#include <BWTA.h>

class IntelManager
{
public:
	static WorkerUnit* scout;
	static BWTA::BaseLocation* gotoLocation;
	static BWTA::BaseLocation* enemyBaselocation;
	static BWTA::Polygon enemyRegionCircle;
	static bool goingToLocation;
	static bool firstpos;
	static void onStart();
	static void hireScout();
	static void onFrame();


	/////////////////////////////////////////////////////////
	static bool highX;
	static bool highY;
	//static UnitStatus scout;
	static BWAPI::Unit scouter;
	static void StartScouting();
	static void ScoutOnFrame(); 
	static std::set<BWTA::BaseLocation*> remainingLocations;
	static int initBaseCount;
	static bool scouting;
};