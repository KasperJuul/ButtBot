#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <iostream>
#include <set>
#include "CostumUnit.h"

using namespace BWAPI;

class BuildingPlacer
{
public:

	static bool supplyProviderIsBeingBuild;
	static bool xpandIsBeingBuild;
	static bool pylonIsInProgress;
	static void onStart();
	static void onFrame();
	static void createType(BWAPI::UnitType unit);
	static BWAPI::TilePosition getBuildTile(UnitType buildingType, TilePosition aroundTile);
	static BWAPI::TilePosition naturalExpantion();
	static BWAPI::TilePosition expantion();
	static void buildOrTrain(UnitType ut);
	static void hireBuilder(UnitType ut);
	static void releaseBuilder(WorkerUnit* w);
	static void builderStateMachine();
	static std::set<WorkerUnit*> BuildingPlacer::builders;

};

