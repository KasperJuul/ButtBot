#pragma once
#include "InformationManager.h"
#include <BWTA.h>

class IntelManager
{
public:
	static bool highX;
	static bool highY;
	static UnitStatus scout;
	static BWAPI::Unit scouter;
	static void StartScouting();
	static void ScoutOnFrame(); 
	static std::set<BWTA::BaseLocation*> remainingLocations;
	static int initBaseCount;
	static bool scouting;
};