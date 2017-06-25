#pragma once
#include "ResourceManager.h"
#include "InformationManager.h"
#include "BuildingPlacer.h"
#include "Debug.h"
#include "IntelManager.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

using namespace BWAPI;
using namespace Filter;


std::string ResourceManager::log = "";
static int miningTimeConstant = 80;
std::vector<ResourceManager::mineralPatch*> ResourceManager::minPatches;


void ResourceManager::onStart(){
	findMinPatches();
	
}

void ResourceManager::onFrame(){

	qGather();
	gasGather();
	//stdGather();
	//buildPylonsNProbes();
}

void ResourceManager::findMinPatches(){
	int itr = 0;
	for (auto m : Broodwar->getMinerals()){
		//if (InformationManager::mainBase->getRegion()->getPolygon().isInside(m->getPosition())){
			mineralPatch* temp = new mineralPatch;
			temp->unit = m;
			temp->name = "M" + std::to_string(itr);
			temp->workers.clear();
			minPatches.push_back(temp);
			itr++;
		//}
	}
}

void ResourceManager::addMinPatches( Unit unit){
	int itr = minPatches.size();
	BWAPI::Region centerRegion = unit->getRegion();
	for (auto m : centerRegion->getUnits(Filter::IsMineralField)){
		mineralPatch* temp;
		temp->unit = m;
		temp->name = "M" + std::to_string(itr);
		temp->workers.clear();
		minPatches.push_back(temp);
		itr++;
	}
}


void ResourceManager::stdGather(){
	for (auto w : InformationManager::workerUnits){
		if (!w->unit->isConstructing() && !w->unit->isGatheringMinerals()){
			w->unit->gather(w->unit->getClosestUnit(IsMineralField || IsRefinery));
		}
	}
}

void ResourceManager::gasGather(){
	if (Broodwar->self()->completedUnitCount(InformationManager::ourRace.getRefinery()) > 0){
		int gasworkers = 0;
		for (auto w : InformationManager::workerUnits){
			if (w->unit->isConstructing()){ continue; }
			if (w->gasworker){
				gasworkers++;
			}
			else if (gasworkers < 3){
				if (!w->builder && !w->isScout){
					if (w->inQ){
						for (unsigned int i = 0; i < w->mineral->workers.size(); i++){
							if (w->mineral->workers.at(i) = w->unit){
								w->mineral->workers.erase(w->mineral->workers.begin() + i);
								w->inQ = false;
							}
						}
					}
					w->unit->gather(w->unit->getClosestUnit(IsRefinery));
					w->state = 0;
					w->gasworker = true;
					w->returningCargo = false;
					gasworkers++;
				}
				
			}
		}
	}

}

void ResourceManager::qGather(){
	for (auto w : InformationManager::workerUnits){

		if (w->gasworker || w->builder){
			if (w->inQ){
				for (unsigned int i = 0; i < w->mineral->workers.size(); i++){
					if (w->mineral->workers.at(i) = w->unit){
						w->mineral->workers.erase(w->mineral->workers.begin() + i);
						w->inQ = false;
					}
				}
			}
			continue;
		}


		if (IntelManager::scout != NULL){
			if (w->unit == IntelManager::scout->unit){
				continue;
			}
		}

		if (w->mineral != NULL){
			if (w->mineral->workers.empty() && w->inQ){
				w->state = 0;
				w->inQ = false;
			}
		}

		switch (w->state){
		case 0 :	// Initial state: Not in a queue and not doing anything
			w->returningCargo = false;
			log += "Calculating round trip for Worker[" + std::to_string(w->unit->getID()) + "] : \n";
			w->mineral = roundTrip_min(w->unit, &minPatches);
			w->mineral->workers.push_back(w->unit);
			w->inQ = true;
			log += "Worker[" + std::to_string(w->unit->getID()) + "] queued at " + w->mineral->name + "\n";
			w->state = 1;
			break;
		case 1:		// Assigned to a queue and checking wether it is first in line or not
			w->unit->gather(w->mineral->unit);
			if (w->unit == w->mineral->workers.front()){
				w->state = 2;
			}
			else {
				w->state = 6;
			}
			break;
		case 2:		// First in line and is movint towards the minerall patch
			if (!w->unit->isMoving()){
				w->state = 3;
			}
			break;
		case 3:		// Is currently gathering from the mineral patch
			
			if (w->unit->isCarryingMinerals()){
				w->unit->returnCargo();
				w->returningCargo = true;
				//for (unsigned int i = 0; i < w->mineral->workers.size(); i++){
				//	if (w->mineral->workers.at(i) = w->unit){
				//		
				//		w->mineral->workers.erase(w->mineral->workers.begin() + i);
				//		w->inQ = false;
				//	}
				//}
				w->mineral->workers.pop_front();
				w->inQ = false;
				w->state = 4;
			}
			if (w->unit->isIdle()){
				w->state = 1;
			}
			//else if (!w->unit->isGatheringMinerals()){
			//	if (!w->unit->gather(w->mineral->unit)){
			//		w->state = 4;
			//	}
			//}
			break;
		case 4:		// Have aquired a mineral and is now returning it
			if (!w->unit->isCarryingMinerals()){
				w->returningCargo = false;
				w->state = 5;
			}
			break;
		case 5:		// Has delivered the mineral
			w->unit->stop();
			w->state = 0;
			break;
		case 6:		// Moving towards the mineral patch to wait or gather if it becomes the first in line on the way
			if (w->unit->getDistance(w->mineral->unit) < 3*32){
				w->unit->stop();
				w->state = 7;
			}
			else if (w->unit == w->mineral->workers.front()){
				w->unit->gather(w->mineral->unit);
				w->state = 2;
			}
			else {
				w->unit->gather(w->mineral->unit);
			}
			break;
		case 7:		// Wait until its your turn to mine
			w->unit->gather(w->mineral->unit);
			if (w->unit == w->mineral->workers.front()){
				w->unit->gather(w->mineral->unit);
				w->state = 2;
			}
			break;
		}
	}
}

// Waiting time and Roundtrip calcutaions
float ResourceManager::workTime(mineralPatch m){
	float w = 0;
	for (unsigned int i = 0; i < m.workers.size(); i++){
		w += workTime(m.workers[i], m , i);
	}
	return w;
}

float ResourceManager::workTime(Unit unit, mineralPatch m, int n){
	float sum = 0;
	if (n > 0){
		for (int i = 0; i <= n - 1; i++){
			sum += workTime2(m.workers[i], m, i);
		}
	}
	float dist = (float)((unit->getDistance(m.unit)) / (unit->getType().topSpeed()));
	float W = std::max(0.0f, dist - sum) + miningTimeConstant;
	//log += "	Work(W) = MAX(0, " + std::to_string(dist) + " - " + std::to_string(sum) + ") + " + std::to_string(miningTimeConstant) + " = " + std::to_string(W) + "\n";
	return W;
}

float ResourceManager::workTime2(Unit unit, mineralPatch m, int n){
	float sum = 0;
	if (n > 0){
		for (int i = 0; i <= n - 1; i++){
			sum += workTime2(m.workers[i], m, i);
		}
	}
	float dist = (float)((unit->getDistance(m.unit)) / (unit->getType().topSpeed()));
	return std::max(0.0f, dist - sum) + miningTimeConstant;
}

float ResourceManager::roundTrip(Unit unit, mineralPatch* m){
	float distance_UtoM = (float)((unit->getDistance(m->unit)) * 4 / (unit->getType().topSpeed()));
	float distance_MtoD = (float)((m->unit)->getDistance(InformationManager::firstCenter) / (unit->getType().topSpeed()));
	for (auto c : InformationManager::productionBuildings){
		if (c->unit->getType().isResourceDepot()){
			float tmp_dist = (float)((m->unit)->getDistance(c->unit) / (unit->getType().topSpeed()));
			if (tmp_dist < distance_MtoD){
				distance_MtoD = tmp_dist;
			}

		}
	}
	
	float derp = workTime(*m);
	float R = distance_UtoM + std::max(0.0f, derp - distance_UtoM) + miningTimeConstant + distance_MtoD;

	//log += "	R(W,M) = " + std::to_string(distance_UtoM) + " + MAX(0, " + std::to_string(derp) + " - " + std::to_string(distance_UtoM) + ") + " + std::to_string(miningTimeConstant) + " + " + std::to_string(distance_MtoD) + " = " + std::to_string(R) + "\n";
	return R;
}

ResourceManager::mineralPatch* ResourceManager::roundTrip_min(Unit unit, std::vector<ResourceManager::mineralPatch*> *patches){
	float time = FLT_MAX;
	int patchItr = 0;
	int itr = 0;
	for (auto m : *patches){
		if (m->workers.size() > 2){
			itr++;
			continue;
		}
		float tripTime = roundTrip(unit, m);
		if (tripTime < time){
			time = tripTime;
			patchItr = itr;
			
		}
		itr++;
	
	}
	return patches->at(patchItr);
}

///////////////// OLD STUFF //////////////////////////////////////////

void ResourceManager::drawMinCircles(){
	for (const auto& mineral : minPatches) {
		Broodwar->drawCircleMap(mineral->unit->getInitialPosition(), 30, Colors::Cyan);
	}
}

void ResourceManager::buildPylonsNProbes(){
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			if (InformationManager::workerUnits.size() > 18){

			}
			// Order the depot to construct more workers! But only when it is idle.
			else if (u->isIdle() && !u->train(u->getType().getRace().getWorker()))
			{
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
				Position pos = u->getPosition();
				Error lastErr = Broodwar->getLastError();
				Broodwar->registerEvent([pos, lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run

				// Retrieve the supply provider type in the case that we have run out of supplies
				UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
				static int lastChecked = 0;

				// If we are supply blocked and haven't tried constructing more recently
				if (lastErr == Errors::Insufficient_Supply &&
					lastChecked + 400 < Broodwar->getFrameCount() &&
					Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
				{
					lastChecked = Broodwar->getFrameCount();

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
						(IsIdle || IsGatheringMinerals) &&
						IsOwned);
					// If a unit was found
					if (supplyBuilder)
					{
						if (supplyProviderType.isBuilding())
						{
							TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
							if (targetBuildLocation)
							{
								// Register an event that draws the target build location
								Broodwar->registerEvent([targetBuildLocation, supplyProviderType](Game*)
								{
									Broodwar->drawBoxMap(Position(targetBuildLocation),
										Position(targetBuildLocation + supplyProviderType.tileSize()),
										Colors::Blue);
								},
									nullptr,  // condition
									supplyProviderType.buildTime() + 100);  // frames to run

								// Order the builder to construct the supply structure
								supplyBuilder->build(supplyProviderType, targetBuildLocation);
							}
						}
						else
						{
							// Train the supply provider (Overlord) if the provider is not a structure
							supplyBuilder->train(supplyProviderType);
						}
					} // closure: supplyBuilder is valid
				} // closure: insufficient supply
			} // closure: failed to train idle unit

		}
	}
}