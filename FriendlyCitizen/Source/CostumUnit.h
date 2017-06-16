#pragma once
#include <BWAPI.h>
#include "UnitState.h"
#include "OwnerProcess.h"
#include "ResourceManager.h"
#include "CostumType.h"

class CostumUnit
{
public:
	BWAPI::Unit unit;
	CostumType type = CostumType::OTHER;
};

class ProductionBuilding : public CostumUnit {
public:
	ProductionBuilding(){
		type = CostumType::PRODUCER;
	}

};

class TechBuilding : public CostumUnit {
public:
	TechBuilding(){
		type = CostumType::TECH;
	}
};

class MilitaryBuilding : public CostumUnit {
public:
	MilitaryBuilding(){
		type = CostumType::DEFENDER;
	}
};

class MilitaryUnit : public CostumUnit {
public:
	int placement = -1;
	MilitaryUnit(){
		type = CostumType::ATTACKER;
	}
};

class WorkerUnit : public CostumUnit {
public:
	WorkerUnit(){
		type = CostumType::WORKER;
	}

	BWAPI::Unit center;
	bool inQ = false;
	bool returningCargo = false;
	bool gasworker = false;
	bool builder = false;
	bool contract = false;
	bool isScout = false;
	bool firstTimeZero = true;
	BWAPI::TilePosition buildTarget = BWAPI::TilePositions::None;
	ResourceManager::mineralPatch* mineral;
	BWAPI::UnitType buildingProject = BWAPI::UnitTypes::None;
	int state = 0;
	UnitState unitState = UnitState::FREE;
	OwnerProcess owner = OwnerProcess::FREE;
};
