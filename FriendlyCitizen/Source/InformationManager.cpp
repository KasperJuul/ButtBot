#pragma once
#include "InformationManager.h"

using namespace BWAPI;

Race InformationManager::ourRace;
Unit InformationManager::firstNexus; //Swap out with better, generalized functionality later
std::vector<Unit> InformationManager::firstWorkers; //Swap out with better, generalized functionality later

void InformationManager::StartAnalysis(){
	InformationManager::ourRace = Broodwar->self()->getRace(); //Gets our current race

	UnitType::set totalSet = InformationManager::ourRace.getWorker().buildsWhat();//Tech tree code start
	bool newFound = true;
	while (newFound){
		bool temp = false;
		int totalSetSizePrior = totalSet.size();
		for (auto b : totalSet){
			totalSet.insert(b.buildsWhat().begin(), b.buildsWhat().end());
		}
		if (totalSet.size() == totalSetSizePrior) newFound = false;
	}

	UnitType baseTech = InformationManager::ourRace.getCenter(); //Unfinished tech tree code

	for (auto &u : Broodwar->self()->getUnits()){
		if (InformationManager::ourRace.getCenter() == u->getType()){
			InformationManager::firstNexus = u;
		}
		else if (InformationManager::ourRace.getWorker() == u->getType()){
			InformationManager::firstWorkers.push_back(u);
		}
	}
	std::string fuckbwapi = InformationManager::firstNexus->getType().getName();
	//debug
	Broodwar->sendText(fuckbwapi.c_str());
}