#pragma once
#include "IntelManager.h"


bool IntelManager::highX = false;
bool IntelManager::highY = false;
//UnitStatus IntelManager::scout;
BWAPI::Unit IntelManager::scouter;
bool IntelManager::scouting = false;
int IntelManager::initBaseCount;
std::set<BWTA::BaseLocation*> IntelManager::remainingLocations;
WorkerUnit* IntelManager::scout;
BWTA::BaseLocation* IntelManager::gotoLocation;
BWTA::BaseLocation* IntelManager::enemyBaselocation;
BWTA::Polygon IntelManager::enemyRegionCircle;
bool IntelManager::goingToLocation;
bool IntelManager::firstpos;

BWAPI::Position pos;
int itr = 0;

void IntelManager::hireScout(){
	for (auto w : InformationManager::workerUnits){
		if (!w->unit->isCarryingMinerals()){
			scout = w;
			for (unsigned int i = 0; i < scout->mineral->workers.size(); i++){
				if (scout->mineral->workers.at(i) = scout->unit){
					scout->mineral->workers.erase(scout->mineral->workers.begin() + i);
					scout->inQ = false;
					scout->returningCargo = false;
					scout->state = 0;
				}
			}
		}
	}
}

void IntelManager::onStart(){
	IntelManager::remainingLocations = BWTA::getStartLocations();
	IntelManager::remainingLocations.erase(BWTA::getStartLocation(BWAPI::Broodwar->self()));
	goingToLocation = false;


}

void IntelManager::onFrame(){
	if (scout == NULL){
		hireScout();
		return;
	}

	switch (scout->state){
	case 0 :	// Initial State
		if (scout->unit->isCarryingGas() || scout->unit->isCarryingMinerals()){
			scout->unit->returnCargo();
			scout->state = 1;
		}
		else {
			scout->state = 2;
		}
		break;
	case 1 :	// Return Cargo if you have some
		if (!scout->unit->isCarryingGas() && !scout->unit->isCarryingMinerals()){
			scout->state = 2;
		}
		break;
	case 2:		// Go to a starting base location
		if (!goingToLocation){
			if (!remainingLocations.empty()){
				int distance = 9000;
				for (auto u : IntelManager::remainingLocations){
					if (scout->unit->getDistance(u->getPosition()) < distance){
						gotoLocation = u;
					}
				}
				scout->unit->move(gotoLocation->getPosition());
				goingToLocation = true;
			}
		}
		if (scout->unit->getDistance(gotoLocation->getPosition() + Position(64, 48)) < 192){
			scout->unit->stop();
			goingToLocation = false;
			remainingLocations.erase(gotoLocation);
			if (scout->unit->getClosestUnit(Filter::IsEnemy, 128)){
				enemyBaselocation = gotoLocation;
				enemyRegionCircle = enemyBaselocation->getRegion()->getPolygon();
				pos = enemyRegionCircle.getNearestPoint(scout->unit->getPosition());
				itr = 0;
				for (auto p : enemyRegionCircle){
					if (p == pos){
						break;
					}
					itr++;
				}
				scout->state = 3;
			}
		}
		break;
	case 3:		// Attack something at enemybase and circle the region to kite/Harass
		if (scout->unit->isAttacking()){
			scout->unit->move(enemyRegionCircle.getNearestPoint(pos));
			scout->state = 4;
		}

		break;
	case 4:		// Circle the region to kite enemy units
		itr = itr % (enemyRegionCircle.size() - 1);
		if (scout->unit->getDistance(enemyRegionCircle.at(itr)) < 160 || !scout->unit->isMoving()){
			itr++;
			itr = itr % (enemyRegionCircle.size() - 1);
			scout->unit->move(enemyRegionCircle.at(itr));
		}
		break;

	}


}