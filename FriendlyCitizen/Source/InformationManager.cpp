#pragma once
#include "InformationManager.h"
#include <string>
#include <set>

using namespace BWAPI;

//######################### REFACTOR #########################################
std::vector<CostumUnit*> InformationManager::costumUnits;
std::vector<ProductionBuilding*> InformationManager::productionBuildings;
std::vector<TechBuilding*> InformationManager::techBuildings;
std::vector<MilitaryBuilding*> InformationManager::militaryBuildings;
std::vector<MilitaryUnit*> InformationManager::militaryUnits;
std::vector<WorkerUnit*> InformationManager::workerUnits;

//############################################################################


Race InformationManager::ourRace;
Unit InformationManager::firstCenter; //Swap out with better, generalized functionality later
std::vector<Unit> InformationManager::firstWorkers; //Swap out with better, generalized functionality later
BWTA::BaseLocation* InformationManager::mainBase;
std::vector<workerUnit> InformationManager::wrkUnits;
std::vector<UnitType> InformationManager::orderedBuildings;
std::vector<EnemyUnit> InformationManager::enemyUnits;

int InformationManager::reservedMinerals;
int InformationManager::reservedGas;
bool InformationManager::enemyFound;
bool InformationManager::enemyBaseFound;
bool InformationManager::initialScoutDestroyed;
bool InformationManager::scout_enable = false;
std::vector<TechNode> InformationManager::ourTech;
Race InformationManager::theirRace;
std::vector<TechNode> InformationManager::theirTech;
std::vector<BWTA::BaseLocation*> InformationManager::baseLocations;
//std::set<UnitStatus> InformationManager::ourUnits; //Catalogues the units we have
std::set<UnitType> InformationManager::ourUnitTypes; //Catalogues the unittypes we have
std::vector<Upgrade*> InformationManager::upgradeList; 
std::vector<Ability*> InformationManager::abilityList;

std::vector<UnitType> InformationManager::enemyUnitTypes;
std::vector<Upgrade*> InformationManager::enemyUpgradeList;
std::vector<Ability*> InformationManager::enemyAbilityList;

std::vector<RegionStruct> InformationManager::regions;

void InformationManager::firstEncounter(BWAPI::Race theirRace){
	InformationManager::theirRace = theirRace;

	//Inital set of what a worker can build
	std::set<UnitType> initialSet;
	initialSet.insert(InformationManager::theirRace.getWorker());

	if (InformationManager::theirRace.c_str() == Races::Zerg.c_str()){
		initialSet.insert(UnitTypes::Zerg_Larva);
	}

	bool newElementAdded = true;
	while (newElementAdded){
		int PriorSetSize = initialSet.size();
		for (auto b : initialSet){
			initialSet.insert(b.buildsWhat().begin(), b.buildsWhat().end());
		}
		if (initialSet.size() == PriorSetSize) newElementAdded = false;
	}

	for (auto t : initialSet){
		TechNode tempNode;
		tempNode.selfType = t;
		if (t == InformationManager::theirRace.getCenter()){
			tempNode.exists = true;
		}
		else if (t == UnitTypes::Zerg_Larva){
			tempNode.exists = true;
		}
		else if (t == UnitTypes::Zerg_Overlord){
			tempNode.exists = true;
		}
		else if (t == InformationManager::ourRace.getWorker()){
			tempNode.exists = true;
		}
		else {
			tempNode.exists = false;
		}
		InformationManager::theirTech.push_back(tempNode);
	}

	//Vectors containing technologies and abilities.
	for (auto u : InformationManager::theirTech){
		for (auto &t : u.selfType.upgradesWhat()){
			Upgrade* temp = new Upgrade;
			temp->selfType = t;
			bool exists = false;
			for (auto t2 : InformationManager::enemyUpgradeList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::enemyUpgradeList.push_back(temp);
		}
	}
	for (auto u : InformationManager::theirTech){
		for (auto &t : u.selfType.researchesWhat()){
			Ability* temp = new Ability;
			temp->selfType = t;
			if (t.requiredUnit() == BWAPI::TechTypes::None){
				temp->researched = true;
			}
			bool exists = false;
			for (auto t2 : InformationManager::enemyAbilityList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::enemyAbilityList.push_back(temp);
		}
		for (auto &t : u.selfType.abilities()){
			Ability* temp = new Ability;
			temp->selfType = t;
			if (t.requiredUnit() == BWAPI::TechTypes::None){
				temp->researched = true;
			}
			bool exists = false;
			for (auto t2 : InformationManager::enemyAbilityList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::enemyAbilityList.push_back(temp);
		}
	}


	//Connect the building/unit tech graph.
	//For every node, we want to fill up the node's effect and precon vectors
	for (unsigned int i = 0; i < InformationManager::theirTech.size(); i++){
		for (auto u : InformationManager::theirTech.at(i).selfType.buildsWhat()){//For each object that the current node can build..
			for (unsigned int i2 = 0; i2 < InformationManager::theirTech.size(); i2++){//We try to find its corresponding node
				if (InformationManager::theirTech.at(i2).selfType == u){
					InformationManager::theirTech.at(i).effect.push_back(&InformationManager::theirTech.at(i2));
				}
			}
		}
		for (auto u : InformationManager::theirTech.at(i).selfType.requiredUnits()){
			for (unsigned int i2 = 0; i2 < InformationManager::theirTech.size(); i2++){
				if (InformationManager::theirTech.at(i2).selfType == u.first){
					InformationManager::theirTech.at(i).precondition.push_back(&InformationManager::theirTech.at(i2));
				}
			}
		}
		if (InformationManager::theirTech.at(i).selfType.producesLarva()){
			for (unsigned int i2 = 0; i2 < InformationManager::theirTech.size(); i2++){//We try to find its corresponding node
				if (InformationManager::theirTech.at(i2).selfType == UnitTypes::Zerg_Larva){
					InformationManager::theirTech.at(i).effect.push_back(&InformationManager::theirTech.at(i2));
				}
			}
		}
	}
}

//Basic bwapi function implementations
//Needs to be refactored
void InformationManager::StartAnalysis(){//Initializes informationmanager
	//Initialization - Algorithmic bools
	InformationManager::enemyFound = false;
	InformationManager::enemyBaseFound = false;
	InformationManager::initialScoutDestroyed = false;

	//Basic data
	reservedMinerals = 0;
	reservedGas = 0;
	InformationManager::ourRace = Broodwar->self()->getRace(); //Gets our current race

	InformationManager::makeTechGraph();

	for (auto &u : Broodwar->self()->getUnits()){//Early functionality to quickly get vital data for other sections of the code
		if (InformationManager::ourRace.getCenter() == u->getType()){
			InformationManager::firstCenter = u;
		}
		else if (InformationManager::ourRace.getWorker() == u->getType()){
			InformationManager::firstWorkers.push_back(u);
		}
	}

	//Adding units to our manual structs.
	for (auto u : Broodwar->self()->getUnits()){
		UnitStatus temp;
		temp.owner = OwnerProcess::FREE;
		temp.self = u;
		temp.state = UnitState::FREE;
//		InformationManager::ourUnits.insert(temp);
		InformationManager::ourUnitTypes.insert(u->getType());
	}

	//Getting the baselocations
	mainBase = BWTA::getStartLocation(Broodwar->self());
	for (auto &b : BWTA::getBaseLocations()){
		if (b == mainBase){ continue;}
		baseLocations.push_back(b);
	}
}

void InformationManager::makeTechGraph(){
	//Inital set of what a worker can build
	std::set<UnitType> initialSet;
	initialSet.insert(InformationManager::ourRace.getWorker());

	if (InformationManager::ourRace.c_str() == Races::Zerg.c_str()){
		initialSet.insert(UnitTypes::Zerg_Larva);
	}

	bool newElementAdded = true;
	while (newElementAdded){
		int PriorSetSize = initialSet.size();
		for (auto b : initialSet){
			initialSet.insert(b.buildsWhat().begin(), b.buildsWhat().end());
		}
		if (initialSet.size() == PriorSetSize) newElementAdded = false;
	}

	for (auto t : initialSet){
		TechNode tempNode;
		tempNode.selfType = t;
		if (t == InformationManager::ourRace.getCenter()){
			tempNode.exists = true;
		}
		else if (t == UnitTypes::Zerg_Larva){
			tempNode.exists = true;
		}
		else if (t == UnitTypes::Zerg_Overlord){
			tempNode.exists = true;
		}
		else if (t == InformationManager::ourRace.getWorker()){
			tempNode.exists = true;
		}
		else {
			tempNode.exists = false;
		}
		InformationManager::ourTech.push_back(tempNode);
	}

	//Vectors containing technologies and abilities.
	for (auto u : ourTech){
		for (auto &t : u.selfType.upgradesWhat()){
			Upgrade* temp = new Upgrade;
			temp->selfType = t;
			bool exists = false;
			for (auto t2 : InformationManager::upgradeList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::upgradeList.push_back(temp);
		}
	}
	for (auto u : ourTech){
		for (auto &t : u.selfType.researchesWhat()){
			Ability* temp = new Ability;
			temp->selfType = t;
			if (t.requiredUnit() == BWAPI::TechTypes::None){
				temp->researched = true;
			}
			bool exists = false;
			for (auto t2 : InformationManager::abilityList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::abilityList.push_back(temp);
		}
		for (auto &t : u.selfType.abilities()){
			Ability* temp = new Ability;
			temp->selfType = t;
			if (t.requiredUnit() == BWAPI::TechTypes::None){
				temp->researched = true;
			}
			bool exists = false;
			for (auto t2 : InformationManager::abilityList){
				if (t2->selfType.getName() == temp->selfType.getName()){
					exists = true;
				}
			}
			if (exists){
				continue;
			}
			InformationManager::abilityList.push_back(temp);
		}
	}

	
	//Connect the building/unit tech graph.
	//For every node, we want to fill up the node's effect and precon vectors
	for (unsigned int i = 0; i < InformationManager::ourTech.size(); i++){
		for (auto u : InformationManager::ourTech.at(i).selfType.buildsWhat()){//For each object that the current node can build..
			for (unsigned int i2 = 0; i2 < InformationManager::ourTech.size(); i2++){//We try to find its corresponding node
				if (InformationManager::ourTech.at(i2).selfType == u){
					InformationManager::ourTech.at(i).effect.push_back(&InformationManager::ourTech.at(i2));
				}
			}
		}
		for (auto u : InformationManager::ourTech.at(i).selfType.requiredUnits()){
			for (unsigned int i2 = 0; i2 < InformationManager::ourTech.size(); i2++){
				if (InformationManager::ourTech.at(i2).selfType == u.first){
					InformationManager::ourTech.at(i).precondition.push_back(&InformationManager::ourTech.at(i2));
				}
			}
		}
		/*if (InformationManager::ourTech.at(i).selfType.producesLarva()){
			for (unsigned int i2 = 0; i2 < InformationManager::ourTech.size(); i2++){//We try to find its corresponding node
				if (InformationManager::ourTech.at(i2).selfType == UnitTypes::Zerg_Larva){
					InformationManager::ourTech.at(i).effect.push_back(&InformationManager::ourTech.at(i2));
				}
			}
		}*/
	}
}

void InformationManager::OnNewUnit(Unit unit){//Should only be called by FriendlyCitizen.cpp. Stores data about new unit.
	if (unit->getPlayer() == Broodwar->self()){
		CostumUnit* temp = new CostumUnit();
		temp->unit = unit;
		bool found = false;
		for (auto us : InformationManager::costumUnits){
			if (us->unit == unit) found = true;
		}
		if (!found){
			InformationManager::costumUnits.push_back(temp);
			if (unit->getType().isBuilding()){
				if (unit->canTrain()){//Note: Nexus gets put here.
					ProductionBuilding* temp = new ProductionBuilding();
					temp->unit = unit;
					InformationManager::productionBuildings.push_back(temp);
				}
				else if (unit->canAttack()){
					MilitaryBuilding* temp = new MilitaryBuilding();
					temp->unit = unit;
					InformationManager::militaryBuildings.push_back(temp);
				}
				else {
					TechBuilding* temp = new TechBuilding();
					temp->unit = unit;
					InformationManager::techBuildings.push_back(temp);
				}
			}
			else{
				if (unit->getType().isWorker()){//Workers!
					WorkerUnit* temp = new WorkerUnit();
					temp->unit = unit;
					temp->unitState = UnitState::FREE;
					InformationManager::workerUnits.push_back(temp);
				}
				else{//Support and extractor gets put here for some reason.
					if (temp->unit->getType() != BWAPI::UnitTypes::Zerg_Overlord){
						MilitaryUnit* temp = new MilitaryUnit();
						temp->unit = unit;
						InformationManager::militaryUnits.push_back(temp);
						
					}
				}
			}

			InformationManager::ourUnitTypes.insert(unit->getType());
			for (auto &ot : InformationManager::ourTech){
				if (ot.selfType == unit->getType()){
					ot.exists = true;
				}
			}
		}
	}
	else if (unit->getPlayer() == Broodwar->enemy()){//Doesn't work in anything beyond 1v1 combat

		EnemyUnit enemy;
		enemy.self = unit;
		enemy.selfID = unit->getID();
		enemy.selfType = unit->getType();
		enemy.lastSeen = unit->getPosition();
		enemy.visible = true;
		bool newUnit = true;
		for (auto e : InformationManager::enemyUnits){
			if (e.self->getID() == unit->getID()){
				newUnit = false;
			}
		}
		if (newUnit){
			InformationManager::enemyUnits.push_back(enemy);
		}
	}
	else {//Neutral units.
		//Nothing happens.
	}
}

void InformationManager::regionAnalyze(){
	for (int i = 0; i < regions.size(); i++){
		bool containsEnemy = false;
		bool containsAlly = false;
		for (auto eu : InformationManager::enemyUnits){
			if (eu.selfType.isBuilding()){
				if (BWTA::getRegion(eu.lastSeen) == regions.at(i).self){
					containsEnemy = true;
					break;
				}
			}
		}

		for (auto ou : InformationManager::costumUnits){
			if (ou->unit->getType().isBuilding()){
				if (BWTA::getRegion(ou->unit->getPosition()) == regions.at(i).self){
					containsAlly = true;
					break;
				}
			}
		}
		if (containsAlly && containsEnemy){
			regions.at(i).owner = OwningPlayer::Dispute;
		}
		else if(containsAlly){
			regions.at(i).owner = OwningPlayer::Self;
		}
		else if (containsEnemy){
			regions.at(i).owner = OwningPlayer::Enemy;
		}
		else {
			regions.at(i).owner = OwningPlayer::Neutral;
		}
	}
}

void InformationManager::regionSetup(){
	std::vector<RegionStruct> allRegions;
	for (auto ra : BWTA::getRegions()){
		RegionStruct temp;
		temp.self = ra;
		allRegions.push_back(temp);
	}

	for (int i = 0; i < allRegions.size(); i++){
		for (auto u : InformationManager::costumUnits){
			if (u->unit->getType().isBuilding()){
				if (BWTA::getRegion(u->unit->getPosition()) == allRegions.at(i).self){
					allRegions.at(i).owner = OwningPlayer::Self;
				}
				
			}
		}

		for (int i2 = 0; i2 < allRegions.size(); i2++){
			const bool contains = allRegions.at(i).self->getReachableRegions().find(allRegions.at(i).self) != allRegions.at(i).self->getReachableRegions().end();
			if (i == i2){
				continue;
			}
			if (contains){
				//Connect the region A to region B.
				allRegions.at(i).neighbours.push_back(&allRegions.at(i2));
			}
		}

	}
	regions = allRegions;
	/*std::vector<std::string> debug;
	for (auto r : regions){
		debug.push_back("Section owned by " + std::to_string(r.owner) + " at " + std::to_string(r.self->getCenter().x) + "," + std::to_string(r.self->getCenter().y) );
	}

	Debug::writeLog(debug, "regiondebug", "test");*/
}

void InformationManager::OnUnitDestroy(Unit unit){
	if (unit->getPlayer() == Broodwar->self()){
		int i = 0;
		int j = 0;
		CostumType tempType;
		for (auto u : InformationManager::costumUnits){
			if (u->unit == unit){
				tempType = u->type;
				//if (u->type == CostumType::PRODUCER){
				j = 0;
					for (auto p : InformationManager::productionBuildings){
						if (p->unit == unit){
							InformationManager::productionBuildings.erase(InformationManager::productionBuildings.begin() + j);
						}
						j++;
					}
					j = 0;
					//}
					//else if (u->type == CostumType::TECH){
					for (auto p : InformationManager::techBuildings){
						if (p->unit == unit){
							InformationManager::techBuildings.erase(InformationManager::techBuildings.begin() + j);
						}
						j++;
					}
					j = 0;
					//}
					//else if (u->type == CostumType::DEFENDER){
					for (auto p : InformationManager::militaryBuildings){
						if (p->unit == unit){
							InformationManager::militaryBuildings.erase(InformationManager::militaryBuildings.begin() + j);
						}
						j++;
					}
					j = 0;
					//}
					//else if (u->type == CostumType::WORKER){
					for (auto p : InformationManager::workerUnits){
						if (p->unit == unit){
							InformationManager::workerUnits.erase(InformationManager::workerUnits.begin() + j);
						}
						j++;
					}
					j = 0;
					//}
					// if (u->type == CostumType::ATTACKER){
					for (auto p : InformationManager::militaryUnits){
						if (p->unit == unit){
							InformationManager::militaryUnits.erase(InformationManager::militaryUnits.begin() + j);
						}
						j++;
					}
					j = 0;
					//}

				InformationManager::costumUnits.erase(InformationManager::costumUnits.begin()+i);
				break;
			}
			i++;
		}

		bool hasStillType = false;
		for (auto u : Broodwar->self()->getUnits()){
			if (u->getType() == unit->getType()){
				hasStillType = true;
				break;
			}
		}
		if (!hasStillType){
			InformationManager::ourUnitTypes.erase(unit->getType());
			for (auto ot : InformationManager::ourTech){
				if (ot.selfType == unit->getType()){
					ot.exists = false;
				}
			}
		}
	}
	else if (unit->getPlayer() == Broodwar->enemy()){//Doesn't work in anything beyond 1v1 combat
		std::vector<EnemyUnit> tempVector;
		for (auto e : InformationManager::enemyUnits){
			if (e.selfID != unit->getID()){
				tempVector.push_back(e);
			}
		}
		InformationManager::enemyUnits = tempVector;
	}
	else {//Neutral units.
		//Nothing happens.
		std::vector<EnemyUnit> tempVector;
		for (auto e : InformationManager::enemyUnits){
			if (e.selfID != unit->getID()){
				tempVector.push_back(e);
			}
		}
		InformationManager::enemyUnits = tempVector;
	}
}


//Deprecated
void InformationManager::AssignUnit(UnitStatus unit, UnitState state, OwnerProcess process){
	unit.state = state;
	unit.owner = process;
}