#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <iostream>


class BuildingPlacer
{
public:
	
	static BWAPI::TilePosition getBuildTile(BWAPI::Unit worker, BWAPI::UnitType building);

};

