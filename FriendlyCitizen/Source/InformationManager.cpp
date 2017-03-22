#pragma once
#include "InformationManager.h"
#include <string>

using namespace BWAPI;

Race InformationManager::ourRace;
Unit InformationManager::firstCenter; //Swap out with better, generalized functionality later
std::vector<Unit> InformationManager::firstWorkers; //Swap out with better, generalized functionality later

void InformationManager::StartAnalysis(){
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

	for (int i = 0; i < tempNodes.size(); i++){//For every node, we want to fill up the node's effect and precon vectors
		for (auto u : tempNodes.at(i).selfType.buildsWhat()){//For each object that the current node can build..
			for (int i2 = 0; i2 < tempNodes.size(); i2++){//We try to find its corresponding node
				if (tempNodes.at(i2).selfType == u){
					tempNodes.at(i).effect.push_back(tempNodes.at(i2));
				}
			}
		}
		for (auto u : tempNodes.at(i).selfType.requiredUnits()){
			for (int i2 = 0; i2 < tempNodes.size(); i2++){
				if (tempNodes.at(i2).selfType == u.first){
					tempNodes.at(i).precondition.push_back(tempNodes.at(i2));
				}
			}
		}
		if (tempNodes.at(i).selfType.producesLarva()){
			for (int i2 = 0; i2 < tempNodes.size(); i2++){//We try to find its corresponding node
				if (tempNodes.at(i2).selfType == UnitTypes::Zerg_Larva){
					tempNodes.at(i).effect.push_back(tempNodes.at(i2));
				}
			}
		}
	}

	for (auto &u : Broodwar->self()->getUnits()){//Early functionality to quickly get vital data for other sections of the code
		if (InformationManager::ourRace.getCenter() == u->getType()){
			InformationManager::firstCenter = u;
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
	for (int i = 0; i < tempNodes.size(); i++){
		if (tempNodes.at(i).selfType == InformationManager::ourRace.getWorker()){
			testTech = tempNodes.at(i).effect.at(6).selfType.toString();
			testTech2 = tempNodes.at(i).precondition.at(0).selfType.toString();
		}
	}
	Broodwar->sendText(testTech.c_str());//Should write out the name of something a worker can build
	Broodwar->sendText(testTech2.c_str());//Should write out the command-center or larva if zerg

	for (int i = 0; i < tempNodes.size(); i++){
		std::string temp = "This unit builds:\n";
		for (int i2 = 0; i2 < tempNodes.at(i).effect.size(); i2++){
			temp += tempNodes.at(i).effect.at(i2).selfType.c_str();
			temp += "\n";
		}
		temp += "\nThis unit requires:\n";
		for (int i2 = 0; i2 < tempNodes.at(i).precondition.size(); i2++){
			temp += tempNodes.at(i).precondition.at(i2).selfType.c_str();
			temp += "\n";
		}
		Debug::writeLog(temp.c_str(), tempNodes.at(i).selfType.getName().c_str(), InformationManager::ourRace.getName().c_str());
	}
}