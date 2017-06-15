#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <vector>
#include "Debug.h"
#include "UnitState.h"
#include "OwnerProcess.h"
#include "ResourceManager.h"
#include "CostumUnit.h"
#include <set>

using namespace BWAPI;
enum OwningPlayer{Self,Enemy,Neutral,Dispute};//Based on buildings only. Dispute = buildings of both players. Neutral = no player buildings.


// ################### REFACTOR #########################

//######################################################################################################

struct RegionStruct{
	BWTA::Region* self;
	std::vector<RegionStruct*> neighbours;
	//All data we want to add
	OwningPlayer owner = OwningPlayer::Neutral;

};

struct EnemyUnit{
	Unit self;//This unit (pointer)
	UnitType selfType;
	Position lastSeen;
	bool visible = false;
	int selfID;
};

struct Upgrade{//TODO: Manually keep track of upgrade's effect using a dedicated class for said information. *Sigh*
	int level = 0;
	//std::string selfType = "";
	BWAPI::UpgradeType selfType = BWAPI::UpgradeTypes::None;
};

struct Ability{
	bool researched = false;
	BWAPI::TechType selfType = BWAPI::TechTypes::None;

};

struct TechNode{
	std::vector<TechNode *> precondition; //Points back to nodes that are required for this unit to be build.
	UnitType selfType; //This unit.
	bool exists = false;
	//int nodeCost = INT_MAX; //How many resources will this cost?
	//Technology* technologyPrecondition = NULL;
	//std::vector<Technology*> technologyEffect; //Pointless, seeing that only one unit in the entire game holds this prequisite, ands ince type already holds techEffect values.
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
	static void makeTechGraph();
	static void firstEncounter();
	static std::vector<Upgrade*> upgradeList; //Keeps track of our upgrades. NOTE: No built-in way to keep track up upgrades nor abilities being finished.
	static std::vector<Ability*> abilityList; //Keeps track of our abilities. NOTE Cont.: Manual solution should be made.
	static std::vector<Upgrade*> enemyUpgradeList;
	static std::vector<Ability*> enemyAbilityList;
	static void firstEncounter(BWAPI::Race theirRace);

	//Region analysis
	

	//Information - Dynamic
	static int reservedMinerals;
	static int reservedGas;
	//static std::set<UnitStatus> ourUnits; //Catalogues the units we have **REPLACED BY COSTUM UNITS**
	static std::set<UnitType> ourUnitTypes; //Catalogues the unittypes we have
	static std::vector<EnemyUnit> enemyUnits;//Catalogues enemy units
	static std::vector<UnitType> enemyUnitTypes; //Catalogues what unittypes they probably have


	//Depcrecated - To be refactored.
	static Unit firstCenter; //Swap out with better, generalized functionality later
	static std::vector<Unit> firstWorkers; //Swap out with better, generalized functionality later
	static std::vector<BWTA::BaseLocation*> baseLocations;
	static BWTA::BaseLocation* mainBase;
	static std::vector<workerUnit> wrkUnits;
	static std::vector<Center> centers; 
	static std::vector<UnitType> orderedBuildings;

	//######################### REFACTOR #########################################
	//Information storage functions

	static std::vector<CostumUnit*> costumUnits;
	static std::vector<ProductionBuilding*> productionBuildings;
	static std::vector<TechBuilding*> techBuildings;
	static std::vector<MilitaryBuilding*> militaryBuildings;
	static std::vector<MilitaryUnit*> militaryUnits;
	static std::vector<WorkerUnit*> workerUnits;

	//############################################################################

	//Last week patchwork
	static void regionSetup();
	static std::vector<RegionStruct> regions;
	static void regionAnalyze();
};