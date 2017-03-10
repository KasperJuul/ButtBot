#include "ResourceManager.h"
#include "InformationManager.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

using namespace BWAPI;
using namespace Filter;

BWTA::BaseLocation* ResourceManager::mainBase;

static int miningTimeConstant = 80;
std::vector<ResourceManager::mineralPatch> ResourceManager::minPatches;
std::vector<ResourceManager::workerUnit> ResourceManager::wrkUnits;

static std::vector<Unit> crystals;

void ResourceManager::onStart(){
	mainBase = BWTA::getStartLocation(Broodwar->self());
	findMinPatches();
	/*for (int i = 0; i < crystals.size(); i++){
		crystals.at(i)->getID();		
	}
	*/
	for (int i = 0; i < minPatches.size(); i++){
		Broodwar << minPatches.at(i).unit->getType().toString()<< " " << minPatches.at(i).unit->getID() << std::endl;
	}

	//for (auto &u : Broodwar->self()->getUnits()){
	//	if (u->getType().isWorker() && minPatches.at(0).workers.size() < 2){
	//		minPatches.at(0).workers.push_back(u);
	//	}
	//	else if (u->getType().isWorker()){
	//		minPatches.at(1).workers.push_back(u);
	//	}

	//}
	
}

void ResourceManager::onFrame(){
	//stdGather();
	//queueGather();
	queueManager();
	buildPylonsNProbes();
}

void ResourceManager::findMinPatches(){
	for (auto m : Broodwar->getMinerals()){
		if (mainBase->getRegion()->getPolygon().isInside(m->getPosition())){
			crystals.push_back(m);
			mineralPatch temp;
			temp.unit = m;
			temp.workers.clear();
			minPatches.push_back(temp);
		}
	}
	
}

void ResourceManager::stdGather(){
	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;

		// Ignore the unit if it has one of the following status ailments
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;

		// Ignore the unit if it is in one of the following states
		if (u->isLoaded() || !u->isPowered() || u->isStuck())
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		if (!u->isCompleted() || u->isConstructing())
			continue;


		// Finally make the unit do some stuff!


		// If the unit is a worker unit
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (u->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					//u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it is carrying a powerup such as a flag
				{       					// Harvest from the nearest mineral patch or gas refinery
					if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery)))
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
			} // closure: if idle
		}
	} // closure: unit iterator
}

void ResourceManager::buildPylonsNProbes(){
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
		{

			// Order the depot to construct more workers! But only when it is idle.
			if (u->isIdle() && !u->train(u->getType().getRace().getWorker()))
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

void ResourceManager::drawMinCircles(){
	for (const auto& mineral : minPatches) {
		Broodwar->drawCircleMap(mineral.unit->getInitialPosition(), 30, Colors::Cyan);
	}
}

//Queue maintanance
void ResourceManager::queueManager(){
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (u->getType().isWorker()){
			if (u->isCarryingMinerals() && !u->isGatheringMinerals()){
				u->returnCargo();
			}
			else if (!u->isCarryingMinerals() && !u->isGatheringMinerals()){
				bool notfound = true;
				for (auto m : minPatches){
					for (auto w : m.workers){
						if (w == u){
							notfound = false;
							break;
						}
					}
					if (!notfound){
						break;
					}
				}
				if (notfound){
					mineralPatch* temp = roundTrip_min(u, &minPatches);
					temp->workers.push_back(u);
				}
				
			}
			else if (u->isGatheringMinerals()){
				Unit min = u->getLastCommand().getTarget();
				for (auto m : minPatches){
					if (m.unit == min){
						m.workers.pop_front();
						break;
					}
				}
			}
		}
	}
}

//Queue gather function
/*
void ResourceManager::queueGather(){
	for (auto &u : Broodwar->self()->getUnits())
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;

		// Ignore the unit if it has one of the following status ailments
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;

		// Ignore the unit if it is in one of the following states
		if (u->isLoaded() || !u->isPowered() || u->isStuck())
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		if (!u->isCompleted() || u->isConstructing())
			continue;

		// If the unit is a worker unit
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (!u->isGatheringMinerals() && u->isCarryingMinerals()){
				u->returnCargo();
			}
			else if (!u->isCarryingMinerals() && !u->isGatheringMinerals())
			{
				mineralPatch patch = roundTrip_min(u, minPatches);
				for (auto m : minPatches){
					if (m.unit == patch.unit){
						m.workers.push_back(u);

						break;
					}
				}
				u->move
			}
			else if (u->isGatheringMinerals())
			{
				Unit patch = u->getLastCommand().getTarget();
				for (auto m : minPatches){
					if (m.unit == patch){
						m.workers.pop_front();
						break;
					}
				}
			}



			if (u->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.

				if (u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it is carrying a powerup such as a flag
				{       					// Harvest from the nearest mineral patch or gas refinery
					mineralPatch patch = roundTrip_min(u, minPatches);
					patch.workers.push_back(u);
					u->move(patch);
					u->


				} // closure: has no powerup
			} // closure: if idle
		}
	}
}
*/


// Waiting time and Roundtrip calcutaions
int ResourceManager::workTime(mineralPatch m){
	int w = 0;
	for (int i = 0; i < m.workers.size(); i++){
		w += workTime(m.workers[i], m , i);
	}
	return w;
}

int ResourceManager::workTime(Unit unit, mineralPatch m, int n){
	int sum = 0;
	if (n > 0){
		for (int i = 0; i <= n - 1; i++){
			sum += workTime(m.workers[i], m, i);
		}
	}
	return std::max(0, unit->getDistance(m.unit) - sum) + miningTimeConstant;
}

int ResourceManager::roundTrip(Unit unit, mineralPatch m){
	int distance_UtoM = unit->getDistance(m.unit);
	int distance_MtoD = (m.unit)->getDistance(InformationManager::firstNexus);
	int R = distance_UtoM + std::max(0, workTime(m) - distance_UtoM) + miningTimeConstant + distance_MtoD;
	return R;
}

ResourceManager::mineralPatch* ResourceManager::roundTrip_min(Unit unit, std::vector<ResourceManager::mineralPatch>* patches){
	int time = 1000000;
	int derp = 0;
	int itr = 0;
	for (auto m : *patches){
		int tripTime = roundTrip(unit, m);
		if (tripTime < time){
			time = tripTime;
			derp = itr;
			
		}
		itr++;
	
	}
	return &patches->at(derp);
}
