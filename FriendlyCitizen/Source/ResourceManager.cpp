#include "ResourceManager.h"
#include "InformationManager.h"
#include "Debug.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

using namespace BWAPI;
using namespace Filter;

BWTA::BaseLocation* ResourceManager::mainBase;

std::string ResourceManager::log = "";
static int miningTimeConstant = 80;
std::vector<ResourceManager::mineralPatch> ResourceManager::minPatches;
std::vector<ResourceManager::workerUnit> ResourceManager::wrkUnits;

void ResourceManager::onStart(){
	mainBase = BWTA::getStartLocation(Broodwar->self());
	findMinPatches();
	
}

void ResourceManager::onFrame(){

	//stdGather();
	//queueGather();
	queueManager2();
	//buildPylonsNProbes();
}

void ResourceManager::findMinPatches(){
	int itr = 0;
	for (auto m : Broodwar->getMinerals()){
		if (mainBase->getRegion()->getPolygon().isInside(m->getPosition())){
			mineralPatch temp;
			temp.unit = m;
			temp.name = "M" + std::to_string(itr);
			temp.workers.clear();
			minPatches.push_back(temp);
			itr++;
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

void ResourceManager::buildPylonsNProbes2(){
	if (InformationManager::firstCenter->isIdle() && wrkUnits.size() < 26){
		if (!InformationManager::firstCenter->train(InformationManager::firstCenter->getType().getRace().getWorker())){
			Position pos = InformationManager::firstCenter->getPosition();
			Error lastErr = Broodwar->getLastError();
			Broodwar->registerEvent([pos, lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
				nullptr,    // condition
				Broodwar->getLatencyFrames());  // frames to run
		}
	}
	if ((Broodwar->self()->supplyTotal() / 2) - (Broodwar->self()->supplyUsed() / 2) <= 3){
		if (InformationManager::ourRace == Races::Enum::Protoss || InformationManager::ourRace == Races::Enum::Terran){
			UnitType supplyProviderType = InformationManager::ourRace.getSupplyProvider();
			Unit supplyBuilder = wrkUnits.at(1).unit;
			bool foundUnit = false;
			//for (auto w : wrkUnits){
			//	if (w.status == "Idle"){
			//		supplyBuilder = w.unit;
			//		foundUnit = true;
			//		break;
			//	}
			//}
			if (foundUnit){
				TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
				if (targetBuildLocation){
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
		}
		else{
			UnitType supplyProviderType = InformationManager::ourRace.getSupplyProvider();
			Unit supplyBuilder = InformationManager::firstCenter->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
				(IsIdle || IsGatheringMinerals) &&
				IsOwned);
			if (supplyBuilder){
				// Train the supply provider (Overlord) if the provider is not a structure
				supplyBuilder->train(supplyProviderType);
			}
		}
	}
}

void ResourceManager::buildPylonsNProbes(){
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			if (wrkUnits.size() > 25){
				
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

void ResourceManager::drawMinCircles(){
	for (const auto& mineral : minPatches) {
		Broodwar->drawCircleMap(mineral.unit->getInitialPosition(), 30, Colors::Cyan);
	}
}

void ResourceManager::queueManager2(){
	for (unsigned int i = 0; i < wrkUnits.size(); i++){
		std::string sts = wrkUnits.at(i).status;
		
		if (wrkUnits.at(i).unit->isConstructing()){
			continue;
		}
		if (sts == "Building" && !wrkUnits.at(i).unit->isConstructing()){
			wrkUnits.at(i).status = "Idle";
		}
		if (sts == "Idle"){
			if (wrkUnits.at(i).unit->isCarryingMinerals()){
				wrkUnits.at(i).status = "In Queue";
				continue;
			}
			log += "Calculating round trip for Worker[" + std::to_string(i) + "] : \n";
			mineralPatch* temp = roundTrip_min(wrkUnits.at(i).unit, &minPatches);
			temp->workers.push_back(wrkUnits.at(i).unit);
			wrkUnits.at(i).status = "In Queue";
			wrkUnits.at(i).mineral = temp;
			log += "Worker[" + std::to_string(i) + "] queued at " + temp->name + "\n";
		}
		if (sts == "In Queue"){
			if (wrkUnits.at(i).mineral->workers.front() == wrkUnits.at(i).unit){
				wrkUnits.at(i).unit->gather(wrkUnits.at(i).mineral->unit);
				wrkUnits.at(i).status = "Mining";
			}
			else{
				wrkUnits.at(i).unit->follow(wrkUnits.at(i).mineral->workers.front());
				wrkUnits.at(i).status = "Waiting";
			}
			
		}
		else if (sts == "Waiting"){
			if (wrkUnits.at(i).mineral->workers.front() == wrkUnits.at(i).unit){
				wrkUnits.at(i).unit->gather(wrkUnits.at(i).mineral->unit);
				wrkUnits.at(i).status = "Mining";
			}
			else {
				wrkUnits.at(i).unit->move(wrkUnits.at(i).mineral->workers.front()->getPosition());
			}
		}
		else if (sts == "Mining"){
			if (wrkUnits.at(i).unit->isCarryingMinerals()){
				wrkUnits.at(i).mineral->workers.pop_front();
				wrkUnits.at(i).unit->returnCargo();
				wrkUnits.at(i).status = "Returning Cargo";
			}
		}
		else if (sts == "Returning Cargo"){
			if (!wrkUnits.at(i).unit->isCarryingMinerals()){
				wrkUnits.at(i).unit->stop();
				wrkUnits.at(i).status = "Idle";
			}
			if (wrkUnits.at(i).unit->isIdle()){
				wrkUnits.at(i).unit->returnCargo();
			}
		}
		
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

void ResourceManager::queueGather(){
	for (unsigned int i = 0; i < minPatches.size(); i++){
		if (minPatches.at(i).workers.front()->isGatheringMinerals()){

		}
	}
}


// Waiting time and Roundtrip calcutaions
int ResourceManager::workTime(mineralPatch m){
	int w = 0;
	for (unsigned int i = 0; i < m.workers.size(); i++){
		w += workTime(m.workers[i], m , i);
	}
	return w;
}

int ResourceManager::workTime(Unit unit, mineralPatch m, int n){
	int sum = 0;
	if (n > 0){
		for (int i = 0; i <= n - 1; i++){
			sum += workTime2(m.workers[i], m, i);
		}
	}
	int dist = (int) ((unit->getDistance(m.unit)) / (unit->getType().topSpeed()));
	int W = std::max(0, dist - sum) + miningTimeConstant;
	log += "	Work(W) = MAX(0, " + std::to_string(dist) + " - " + std::to_string(sum) + ") + " + std::to_string(miningTimeConstant) + " = " + std::to_string(W) + "\n";
	return W;
}

int ResourceManager::workTime2(Unit unit, mineralPatch m, int n){
	int sum = 0;
	if (n > 0){
		for (int i = 0; i <= n - 1; i++){
			sum += workTime2(m.workers[i], m, i);
		}
	}
	int dist = (int)((unit->getDistance(m.unit)) / (unit->getType().topSpeed()));
	return std::max(0, dist - sum) + miningTimeConstant;
}

int ResourceManager::roundTrip(Unit unit, mineralPatch m){
	int distance_UtoM = (int)((unit->getDistance(m.unit)) / (unit->getType().topSpeed()));
	int distance_MtoD = (int)((m.unit)->getDistance(InformationManager::firstCenter) / (unit->getType().topSpeed()));
	int derp = workTime(m);
	int R = distance_UtoM + std::max(0, derp - distance_UtoM) + miningTimeConstant + distance_MtoD;

	log += "	R(W,M) = " + std::to_string(distance_UtoM) + " + MAX(0, " + std::to_string(derp) + " - " + std::to_string(distance_UtoM) + ") + " + std::to_string(miningTimeConstant) + " + " + std::to_string(distance_MtoD) + " = " + std::to_string(R) + "\n";
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
