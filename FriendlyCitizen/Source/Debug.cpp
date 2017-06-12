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

void Debug::writeLog(std::vector<std::string> messages, std::string fileName, std::string folderName){//Overload for vectors
	std::string temp = "";
	for (auto sentence : messages){
		temp += sentence + "\n";
	}
	writeLog(temp, fileName, folderName);
}

void Debug::writeLog(std::set<std::string> messages, std::string fileName, std::string folderName){//Overload for sets
	std::string temp = "";
	for (auto sentence : messages){
		temp += sentence + "\n";
	}
	writeLog(temp, fileName, folderName);
}//Improvement: Find a way to iterate over any type of collection.

void Debug::writeTimedLog(std::string message, std::string fileName, std::string folderName){//BUGGY - crashes starcraft.
	_mkdir(folderName.c_str());
	time_t rawTime;
	struct tm * timeInfo;
	time(&rawTime);
	localtime_s(timeInfo, &rawTime);
	std::ofstream writeTo;
	char buffer[26];
	asctime_s(buffer, sizeof buffer, timeInfo);
	writeTo.open(folderName + "\\" + buffer + "_" + fileName + ".txt");
	writeTo << message + "\n";
	writeTo.close();
}

void Debug::screenInfo(){//More complex debugging can be done here in the future.
	for (auto &u : Broodwar->self()->getUnits()){
		if (u->getType().isWorker()){
			Broodwar->drawTextMap(u->getPosition(), "%c%d", Text::Blue, u->getID());
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

	std::string b = "Centers: " + std::to_string(InformationManager::centers.size());
	std::string w = "Workers: " + std::to_string(InformationManager::centers.at(0).wrkUnits.size());
	
	Broodwar->drawTextScreen(20, 0, b.c_str());
	Broodwar->drawTextScreen(20, 10, w.c_str());
	Broodwar->drawTextScreen(20, 30, BuildingPlanner::plan.c_str());

	Broodwar->drawTextScreen(588, 15, "%c%d/%d", Text::Yellow, Broodwar->self()->supplyUsed() / 2, BWAPI::Broodwar->self()->supplyTotal() / 2);
	Broodwar->drawTextScreen(452, 15, "%c%d", Text::Yellow, InformationManager::reservedMinerals);
	Broodwar->drawTextScreen(520, 15, "%c%d", Text::Yellow, InformationManager::reservedGas);

}

void Debug::errorLogMessages(std::string input){
	std::string temp = "Frame count: " + std::to_string(Broodwar->getFrameCount());
	temp += " Msg: " + input;
	Debug::errorLog.push_back(temp);
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