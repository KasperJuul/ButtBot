#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <iostream>
#include <set>

using namespace BWAPI;

class BuildingPlacer
{
public:

	static bool supplyProviderIsBeingBuild;
	static bool xpandIsBeingBuild;
	static void onStart();
	static void onFrame();
	static void createType(BWAPI::UnitType unit);
	static BWAPI::TilePosition getBuildTile(UnitType buildingType, TilePosition aroundTile);
	static BWAPI::TilePosition naturalExpantion();

};

