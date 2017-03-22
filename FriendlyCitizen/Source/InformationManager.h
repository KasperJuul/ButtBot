#pragma once
#include <BWAPI.h>
#include <vector>
#include "Debug.h"

using namespace BWAPI;

static struct TechNode{
	std::vector<TechNode> precondition; //Points back to nodes that are required for this unit to be build.
	UnitType selfType; //This unit.
	bool exists = false;
	//int nodeCost = INT_MAX; //How many resources will this cost?
	std::vector<TechNode> effect; //Points to nodes that can be built by this node.
};

static class InformationManager
{
public:
	//Analytial functions
	static void StartAnalysis();

	//Information
	static Race ourRace;
	//static UnitType::set ourUnitTypes; //Catalogues the unit types that we currently have. To be added later
	//static std::vector<Unit> ourUnits; //Catalogues our units. To be added later
	static Unit firstCenter; //Swap out with better, generalized functionality later
	static std::vector<Unit> firstWorkers; //Swap out with better, generalized functionality later
};

