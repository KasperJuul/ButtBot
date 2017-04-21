#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <vector>
#include "Debug.h"
#include "UnitState.h"
#include "OwnerProcess.h"
#include "ResourceManager.h"
#include <set>

using namespace BWAPI;

struct EnemyUnit{
	Unit self;//This unit (pointer)
	UnitType selfType;
	Position lastSeen;
	int selfID;
};

struct TechNode{
	std::vector<TechNode *> precondition; //Points back to nodes that are required for this unit to be build.
	UnitType selfType; //This unit.
	bool exists = false;
	//int nodeCost = INT_MAX; //How many resources will this cost?
	std::vector<TechNode *> effect; //Points to nodes that can be built by this node.
};

struct UnitStatus{
	UnitState state = UnitState::FREE;
	OwnerProcess owner = OwnerProcess::FREE;
	Unit self;
};

struct workerUnit{
	BWAPI::Unit unit;
	ResourceManager::mineralPatch* mineral;
	TilePosition buildTile;
	UnitType buildUnit;
	std::string status;		// Idle, Mining, Returning Cargo, Waiting,
};

struct Center{
	BWAPI::Unit unit;
	std::vector<workerUnit> wrkUnits;
	std::vector<Unit> barracks;
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
	static int reservedMinerals;
	static int reservedGas;
	static std::set<UnitStatus> ourUnits; //Catalogues the units we have
	static std::set<UnitType> ourUnitTypes; //Catalogues the unittypes we have
	static std::vector<EnemyUnit> enemyUnits;//Catalogues enemy units

	//Depcrecated - To be refactored.
	static Unit firstCenter; //Swap out with better, generalized functionality later
	static std::vector<Unit> firstWorkers; //Swap out with better, generalized functionality later
	static std::vector<BWTA::BaseLocation*> baseLocations;
	static BWTA::BaseLocation* mainBase;
	static std::vector<workerUnit> wrkUnits;
	static std::vector<Center> centers; 
	static std::vector<UnitType> orderedBuildings;
};