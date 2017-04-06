#pragma once
#include "InformationManager.h"
#include <string>
#include <set>

using namespace BWAPI;

Race InformationManager::ourRace;
Unit InformationManager::firstCenter; //Swap out with better, generalized functionality later
std::vector<Unit> InformationManager::firstWorkers; //Swap out with better, generalized functionality later
BWTA::BaseLocation* InformationManager::mainBase;
std::vector<workerUnit> InformationManager::wrkUnits;
std::vector<Center> InformationManager::centers;

bool InformationManager::enemyFound;
bool InformationManager::enemyBaseFound;
bool InformationManager::initialScoutDestroyed;
std::vector<TechNode> InformationManager::ourTech;
Race InformationManager::theirRace;
std::vector<TechNode> InformationManager::theirTech;
std::vector<BWTA::BaseLocation*> InformationManager::baseLocations;
std::set<UnitStatus> InformationManager::ourUnits; //Catalogues the units we have
std::set<UnitType> InformationManager::ourUnitTypes; //Catalogues the unittypes we have

void InformationManager::StartAnalysis(){//Initializes informationmanager
	//Initialization - Algorithmic bools
	InformationManager::enemyFound = false;
	InformationManager::enemyBaseFound = false;
	InformationManager::initialScoutDestroyed = false;

	//Basic data
	InformationManager::ourRace = Broodwar->self()->getRace(); //Gets our current race
	
	UnitType::set totalSet = InformationManager::ourRace.getWorker().buildsWhat();//Tech tree code start

	if (InformationManager::ourRace.c_str() == Races::Zerg.c_str()){
		totalSet.insert(UnitTypes::Zerg_Larva);
	}
	bool newFound = true;
	while (newFound){
		bool temp = false;
		int totalSetSizePrior = totalSet.size();
		for (auto b : totalSet){
			totalSet.insert(b.buildsWhat().begin(), b.buildsWhat().end());
		}
		if (totalSet.size() == totalSetSizePrior) newFound = false;
	}
	UnitType baseTech;
	if (InformationManager::ourRace.c_str() != Races::Zerg.c_str()){
		baseTech = InformationManager::ourRace.getCenter(); //Unfinished tech tree code
	}
	else {
		baseTech = UnitTypes::Zerg_Larva;
	}
	std::vector<TechNode> tempNodes;
	for (auto t : totalSet){
		TechNode tempNode;
		tempNode.selfType = t;
		if (t == InformationManager::ourRace.getCenter()){
			tempNode.exists = true;
		}
		else if(t == UnitTypes::Zerg_Larva){
			tempNode.exists = true;
		}
		else if (t == UnitTypes::Zerg_Overlord){
			tempNode.exists = true;
		}
		else if (t == InformationManager::ourRace.getWorker()){
			tempNode.exists = true;
		}
		tempNodes.push_back(tempNode);
	}

	for (unsigned int i = 0; i < tempNodes.size(); i++){//For every node, we want to fill up the node's effect and precon vectors
		for (auto u : tempNodes.at(i).selfType.buildsWhat()){//For each object that the current node can build..
			for (unsigned int i2 = 0; i2 < tempNodes.size(); i2++){//We try to find its corresponding node
				if (tempNodes.at(i2).selfType == u){
					tempNodes.at(i).effect.push_back(tempNodes.at(i2));
				}
			}
		}
		for (auto u : tempNodes.at(i).selfType.requiredUnits()){
			for (unsigned int i2 = 0; i2 < tempNodes.size(); i2++){
				if (tempNodes.at(i2).selfType == u.first){
					tempNodes.at(i).precondition.push_back(tempNodes.at(i2));
				}
			}
		}
		if (tempNodes.at(i).selfType.producesLarva()){
			for (unsigned int i2 = 0; i2 < tempNodes.size(); i2++){//We try to find its corresponding node
				if (tempNodes.at(i2).selfType == UnitTypes::Zerg_Larva){
					tempNodes.at(i).effect.push_back(tempNodes.at(i2));
				}
			}
		}
	}

	for (auto &u : Broodwar->self()->getUnits()){//Early functionality to quickly get vital data for other sections of the code
		if (InformationManager::ourRace.getCenter() == u->getType()){
			InformationManager::firstCenter = u;
			Center temp;
			temp.unit = u;
			temp.wrkUnits.clear();
			centers.push_back(temp);
		}
		else if (InformationManager::ourRace.getWorker() == u->getType()){
			InformationManager::firstWorkers.push_back(u);
		}
	}

	//debug-init
	std::string testAbove = InformationManager::firstCenter->getType().getName();//Tests if the above works. Manual test: See that the printed name equates the base type
	Broodwar->sendText(testAbove.c_str());

	std::string testTech = tempNodes.at(0).selfType.toString();
	std::string testTech2;
	Broodwar->sendText(testTech.c_str());
	for (unsigned int i = 0; i < tempNodes.size(); i++){
		if (tempNodes.at(i).selfType == InformationManager::ourRace.getWorker()){
			testTech = tempNodes.at(i).effect.at(6).selfType.toString();
			testTech2 = tempNodes.at(i).precondition.at(0).selfType.toString();
		}
	}
	Broodwar->sendText(testTech.c_str());//Should write out the name of something a worker can build
	Broodwar->sendText(testTech2.c_str());//Should write out the command-center or larva if zerg

	for (unsigned int i = 0; i < tempNodes.size(); i++){
		std::string temp = "This unit builds:\n";
		for (unsigned int i2 = 0; i2 < tempNodes.at(i).effect.size(); i2++){
			temp += tempNodes.at(i).effect.at(i2).selfType.c_str();
			temp += "\n";
		}
		temp += "\nThis unit requires:\n";
		for (unsigned int i2 = 0; i2 < tempNodes.at(i).precondition.size(); i2++){
			temp += tempNodes.at(i).precondition.at(i2).selfType.c_str();
			temp += "\n";
		}
		Debug::writeLog(temp.c_str(), tempNodes.at(i).selfType.getName().c_str(), InformationManager::ourRace.getName().c_str());
	}

	InformationManager::ourTech = tempNodes;


	//Adding units to our manual structs.
	for (auto u : Broodwar->self()->getUnits()){
		UnitStatus temp;
		temp.owner = OwnerProcess::FREE;
		temp.self = u;
		temp.state = UnitState::FREE;
		InformationManager::ourUnits.insert(temp);
		InformationManager::ourUnitTypes.insert(u->getType());
	}

	//Getting the baselocations
	mainBase = BWTA::getStartLocation(Broodwar->self());
	for (auto &b : BWTA::getBaseLocations()){
		if (b == mainBase){ continue;}
		baseLocations.push_back(b);
	}
}

void InformationManager::OnNewUnit(Unit unit){//Should only be called by FriendlyCitizen.cpp. Stores data about new unit.
	if (unit->getPlayer() == Broodwar->self()){
		UnitStatus temp;
		temp.owner = OwnerProcess::FREE;
		temp.self = unit;
		temp.state = UnitState::FREE;
		bool found = false;
		for (auto us : InformationManager::ourUnits){
			if (us.self == unit) found = true;
		}
		if (!found){
			InformationManager::ourUnits.insert(temp);
			InformationManager::ourUnitTypes.insert(unit->getType());
			for (auto ot : InformationManager::ourTech){
				if (ot.selfType == unit->getType()){
					ot.exists = true;
				}
			}
		}
	}
	else if(unit->getPlayer() == Broodwar->enemy()){//Doesn't work in anything beyond 1v1 combat
		//Unimplemented
	}
	else {//Neutral units.
		//Unimplemented
	}
}

void InformationManager::OnUnitDestroy(Unit unit){
	if (unit->getPlayer() == Broodwar->self()){
		for (auto u : InformationManager::ourUnits){
			if (u.self == unit){
				InformationManager::ourUnits.erase(u);
				break;
			}
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
		//Unimplemented
	}
	else {//Neutral units.
		//Unimplemented
	}
}

void InformationManager::AssignUnit(UnitStatus unit, UnitState state, OwnerProcess process){
	unit.state = state;
	unit.owner = process;
}