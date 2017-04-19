#include "IntelManager.h"


bool IntelManager::highX = false;
bool IntelManager::highY = false;
UnitStatus IntelManager::scout;
BWAPI::Unit IntelManager::scouter;
bool IntelManager::scouting = false;
int IntelManager::initBaseCount;
std::set<BWTA::BaseLocation*> IntelManager::remainingLocations;

void IntelManager::StartScouting()
{
	Unit scoutUnit;
	for (auto u : BWAPI::Broodwar->self()->getUnits()){
		if (u->getType().isWorker()){
			scoutUnit = u;
			break;
		}
	}
	for (auto u : InformationManager::ourUnits){
		if (scoutUnit == u.self){
			IntelManager::scout = u;
		}
	}
	//Infomanager?
	//InformationManager::ReadUnit(scout);
	//If readunit returns UnitState::RedAlert or other vital states, don't use the unit. Otherwise, do.
	//InformationManager::AssignUnit(scout,UnitState::Scout,OwnerProcess::Intel);
	IntelManager::remainingLocations = BWTA::getStartLocations();
	IntelManager::remainingLocations.erase(BWTA::getStartLocation(BWAPI::Broodwar->self()));
	IntelManager::initBaseCount = IntelManager::remainingLocations.size();
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition().y > Broodwar->mapHeight()/2){
		highY = true;
	}
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition().x > Broodwar->mapWidth() / 2){
		highX = true;
	}

}

void IntelManager::ScoutOnFrame(){
	if (!IntelManager::scouting){
		if (Broodwar->self()->supplyUsed() > 10){//Temporary
			if (IntelManager::scout.self->exists()){//TODO: Refine scout.
				//if (IntelManager::scout.self->isIdle()){ 
				//if (!IntelManager::scout.self->isCarryingMinerals() /*&& !IntelManager::scout.self->isGatheringMinerals()*/){
					InformationManager::AssignUnit(scout, UnitState::WORKING, OwnerProcess::INTEL);
					IntelManager::scouting = true;
				//}
			}
			else {
				Unit scoutUnit;
				for (auto u : Broodwar->self()->getUnits()){
					if (u->getType().isWorker()){
						scoutUnit = u;
						break;
					}
				}
				for (auto u : InformationManager::ourUnits){
					if (scoutUnit == u.self){
						IntelManager::scout = u;
					}
				}
			}
		}
	}
	if (IntelManager::scouting && IntelManager::remainingLocations.size() > 0){
		if (IntelManager::initBaseCount > 2){
			//Clockwise scouting
			for (auto u : IntelManager::remainingLocations){
				IntelManager::scout.self->move(u->getPosition());
				if (IntelManager::scout.self->getType().sightRange() / 2 > IntelManager::scout.self->getDistance(u->getPosition())){
					IntelManager::remainingLocations.erase(u);
				}
				break;
			}
		}
		else {
			//Flippant scouting
			for (auto u : IntelManager::remainingLocations){
				IntelManager::scout.self->move(u->getPosition());
				if (IntelManager::scout.self->getType().sightRange()/2 > IntelManager::scout.self->getDistance(u->getPosition())){
					IntelManager::remainingLocations.erase(u);
				}
				break;
			}
		}
	}
}
