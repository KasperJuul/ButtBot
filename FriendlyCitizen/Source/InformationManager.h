#pragma once
#include <BWAPI.h>
#include <vector>
#include "Debug.h"
#include "UnitState.h"
#include "OwnerProcess.h"
#include <set>

using namespace BWAPI;

struct TechNode{
	std::vector<TechNode> precondition; //Points back to nodes that are required for this unit to be build.
	UnitType selfType; //This unit.
	bool exists = false;
	//int nodeCost = INT_MAX; //How many resources will this cost?
	std::vector<TechNode> effect; //Points to nodes that can be built by this node.
};

struct UnitStatus{
	UnitState state = UnitState::FREE;
	OwnerProcess owner = OwnerProcess::FREE;
	Unit self;
};

inline bool operator<(const UnitStatus& lhs, const UnitStatus& rhs)
{
	return lhs.self->getID() < rhs.self->getID();
}

class InformationManager
{
public:
	//Analytial functions
	static void StartAnalysis();//Constructs a tech tree and stores the initial data for our race
	//static void EnemyAnalysis();//Constructs a tech tree and stores data about the enemy. Currently unimplemented

	//Information storage functions
	static void OnNewUnit(Unit unit);
	static void OnUnitDestroy(Unit unit);

	//Communications functions
	static void AssignUnit(UnitStatus unit, UnitState state, OwnerProcess process);

	//Algorithmic booleans
	static bool enemyFound;
	static bool enemyBaseFound;
	static bool initialScoutDestroyed;

	//Information - Static
	static Race ourRace;
	static std::vector<TechNode> ourTech;
	static Race theirRace;
	static std::vector<TechNode> theirTech;

	//Information - Dynamic
	static std::set<UnitStatus> ourUnits; //Catalogues the units we have
	static std::set<UnitType> ourUnitTypes; //Catalogues the unittypes we have

	//Depcrecated - To be refactored.
	static Unit firstNexus; //Swap out with better, generalized functionality later
	//static UnitType::set ourUnitTypes; //Catalogues the unit types that we currently have. To be added later
	//static std::vector<Unit> ourUnits; //Catalogues our units. To be added later
	static Unit firstCenter; //Swap out with better, generalized functionality later
	static std::vector<Unit> firstWorkers; //Swap out with better, generalized functionality later
};