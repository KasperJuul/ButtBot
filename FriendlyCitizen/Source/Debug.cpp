#pragma once
#include "Debug.h"
#include "InformationManager.h"
#include "BuildingPlanner.h"
#include <fstream>
#include <direct.h>
#include <time.h>

using namespace BWAPI;

std::vector<std::string> Debug::errorLog;

void Debug::writeLog(std::string message, std::string fileName, std::string folderName){
	_mkdir(folderName.c_str());
	std::ofstream writeTo;
	writeTo.open(folderName + "\\" + fileName + ".txt");
	writeTo << message+"\n";
	writeTo.close();
}

void Debug::writeTimedLog(std::string message, std::string fileName, std::string folderName){//UNTESTED.
	_mkdir(folderName.c_str());
	time_t rawTime;
	struct tm * timeInfo;
	time(&rawTime);
	localtime_s(timeInfo, &rawTime);
	std::ofstream writeTo;
	char buffer[26];
	asctime_s(buffer, sizeof buffer, timeInfo);
	writeTo.open(folderName + "\\" + buffer + "; " + fileName + ".txt");
	writeTo << message + "\n";
	writeTo.close();
}

void Debug::screenInfo(){
	for (auto w : InformationManager::workerUnits){
		Broodwar->drawTextMap(w->unit->getPosition(), "%c%d", Text::Yellow, w->unit->getID());
		Broodwar->drawTextMap(w->unit->getPosition() + Position(0, -10), "%c%d", Text::Cyan, w->state);
	}

	for (auto &u : Broodwar->self()->getUnits()){
		if (u->getType().isWorker()){
			continue;
			//Broodwar->drawTextMap(u->getPosition(), "%c%d", Text::Yellow, u->getID());
		}
		else if (u->getType().isResourceDepot()){
			Broodwar->drawTextMap(u->getPosition() + Position(0,-15), "%c%d", Text::Orange, u->getID());
		}
		else{
			Broodwar->drawTextMap(u->getPosition(), "%c%d", Text::BrightRed, u->getID());
		}

	}
	for (auto &u : Broodwar->enemy()->getUnits()){
		if (u->getType().isResourceDepot()){
			Broodwar->drawTextMap(u->getPosition(), "%c%d", Text::Orange, u->getID());
		}
	}

	std::string centers = "Centers: " + std::to_string(InformationManager::centers.size());
	std::string workers = "Workers: " + std::to_string(InformationManager::workerUnits.size());
	
	Broodwar->drawTextScreen(20, 0, centers.c_str());
	Broodwar->drawTextScreen(20, 10, workers.c_str());
	Broodwar->drawTextScreen(20, 30, BuildingPlanner::plan.c_str());

	int some = 0;
	for (auto m :ResourceManager::minPatches){
		std::string minstring = m.name + ": ";
		std::string gathered = "";
		if (!m.workers.empty()){
			for (unsigned int j = 0; j < m.workers.size(); j++){
				minstring += "[" + std::to_string(m.workers.at(j)->getID()) + "] ";
			}
		}
		if (m.unit->isBeingGathered()){
			gathered = "True";
		}
		else {
			gathered = "False";
		}
		Broodwar->drawTextScreen(20, 40 + (some * 10), minstring.c_str());
		Broodwar->drawTextMap(m.unit->getPosition() + Position(-5, -10), "%c%s", Text::Yellow, m.name.c_str());
		Broodwar->drawTextMap(m.unit->getPosition() + Position(-5, -20), "%c%s", Text::Yellow, gathered.c_str());

		some++;
	}

	Broodwar->drawTextScreen(588, 15, "%c%d/%d", Text::Yellow, Broodwar->self()->supplyUsed() / 2, BWAPI::Broodwar->self()->supplyTotal() / 2);
	Broodwar->drawTextScreen(452, 15, "%c%d", Text::Yellow, InformationManager::reservedMinerals);
	Broodwar->drawTextScreen(520, 15, "%c%d", Text::Yellow, InformationManager::reservedGas);

}

void Debug::errorLogMessages(std::string input){
	Debug::errorLog.push_back(input);
}

void Debug::endWriteLog(){
	_mkdir("tests");
	std::ofstream writeTo;
	writeTo.open("tests\\ErrorLog.txt");
	for (auto message : Debug::errorLog){
		writeTo << message + "\n";
	}
	writeTo.close();
}