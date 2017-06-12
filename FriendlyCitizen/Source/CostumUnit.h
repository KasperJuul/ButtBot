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
	ResourceManager::mineralPatch* mineral;
	BWAPI::TilePosition buildTile;
	BWAPI::UnitType buildUnit;
	UnitState state = UnitState::FREE;
	OwnerProcess owner = OwnerProcess::FREE;
};
