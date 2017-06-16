#pragma once
#include "BuildingPlacer.h"
#include "BuildingPlanner.h"
#include "InformationManager.h"
#include "IntelManager.h"
#include "Debug.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace BWTA;

std::set<WorkerUnit*> BuildingPlacer::builders;
bool BuildingPlacer::supplyProviderIsBeingBuild = false; 
bool BuildingPlacer::xpandIsBeingBuild = false;
bool BuildingPlacer::pylonIsInProgress = false;


void BuildingPlacer::onStart(){
	
}

void BuildingPlacer::onFrame(){
	//If someone is a miner and has a plan, errolog
	for (auto w : InformationManager::workerUnits){
		if (!w->builder && w->buildingProject != UnitTypes::None){
			Debug::errorLogMessages("Miner has buildproject and is not a builder!");
		}
	}

	if (InformationManager::reservedMinerals < 0){
		InformationManager::reservedMinerals = 0;
		Debug::errorLogMessages("Reserved minerals negative");
	}
	if (InformationManager::reservedGas < 0){
		InformationManager::reservedGas = 0;
		Debug::errorLogMessages("Reserved gas negative");
	}

	for (auto build : BuildingPlanner::findOrder()){
		if (build.unitType == UnitTypes::None){
			continue;
		}

		bool canAfford = build.unitType.mineralPrice() <= (Broodwar->self()->minerals() - InformationManager::reservedMinerals)
						 && build.unitType.gasPrice() <= (Broodwar->self()->gas() - InformationManager::reservedGas);

		bool canAffordM = build.unitType.mineralPrice() <= (Broodwar->self()->minerals() - InformationManager::reservedMinerals);
		
		if (!canAfford && canAffordM){
			continue;
		}

		if (canAfford){
			if (build.unitType.isAddon()){
				for (auto &u : Broodwar->self()->getUnits()){
					if (u->canBuildAddon(build.unitType) && u->isIdle()){
						if (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() - build.unitType.supplyRequired() >= 0){
							u->buildAddon(build.unitType);
						}
						break;
					}
				}
			}
			else if (InformationManager::ourRace == Races::Zerg && build.unitType.whatBuilds().first.isBuilding()){
				for (auto &u : Broodwar->self()->getUnits()){
					if (u->canMorph(build.unitType) && u->isIdle()){
						if (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() - build.unitType.supplyRequired() >= 0){
							u->morph(build.unitType);
						}
						break;
					}
				}
			}
			else if (build.unitType.isBuilding()){
				bool ip = false;
				for (auto bu : builders){
					if (bu->buildingProject == build.unitType){
						ip = true;
					}
				}
				if (!ip && Broodwar->self()->incompleteUnitCount(build.unitType) < 1){
					hireBuilder(build.unitType);
				}
			}
			else{
				for (auto &u : Broodwar->self()->getUnits()){
					if (u->canTrain(build.unitType) && u->isIdle()){
						if (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() - build.unitType.supplyRequired() >= 0){
							u->train(build.unitType);
						}
						else {
							bool canAfford2 = build.unitType.mineralPrice() <= (Broodwar->self()->minerals() - InformationManager::reservedMinerals)
								&& build.unitType.gasPrice() <= (Broodwar->self()->gas() - InformationManager::reservedGas);
							if (canAfford2){
								hireBuilder(Broodwar->self()->getRace().getSupplyProvider());
							}
						}
						break;
					}
				}
			}
		}
		else{ break; }
	}

	builderStateMachine();
}

void BuildingPlacer::hireBuilder(UnitType ut){
	for (auto w : InformationManager::workerUnits){
		if (!w->unit->isCarryingMinerals() && !w->gasworker && !w->builder && !w->isScout){
			if (w->inQ){
				for (unsigned int i = 0; i < w->mineral->workers.size(); i++){
					if (w->mineral->workers.at(i) = w->unit){
						w->mineral->workers.erase(w->mineral->workers.begin() + i);
						w->inQ = false;
						w->returningCargo = false;
						w->state = 0;
					}
				}
			}
			w->builder = true;
			w->buildingProject = ut;
			w->contract = true;
			builders.insert(w);
			Broodwar << "Builder " << std::to_string(w->unit->getID()) << " is hired to build" << w->buildingProject.toString() << std::endl;
			break;
		}
	}
}

void BuildingPlacer::releaseBuilder(WorkerUnit* w){
	builders.erase(w);
	w->buildingProject = UnitTypes::None;
	w->buildTarget = TilePositions::None;
	w->builder = false;
	w->inQ = false;
	w->state = 0;
	w->gasworker = false;
	w->returningCargo = false;
	w->firstTimeZero = true;
}

void BuildingPlacer::builderStateMachine(){
	for (auto b : builders){
		TilePosition tile = b->buildTarget;
		UnitType unitype = b->buildingProject;

		switch (b->state){
		case 0:	// Initial State - Just hired!
			if (b->firstTimeZero){
				b->buildTarget = getBuildTile(b->buildingProject, Broodwar->self()->getStartLocation());
				InformationManager::reservedGas += b->buildingProject.gasPrice();
				InformationManager::reservedMinerals += b->buildingProject.mineralPrice();
				b->firstTimeZero = false;
			}
			
			if (b->buildTarget == TilePositions::None) {
				if (InformationManager::ourRace == Races::Protoss && !pylonIsInProgress){
					InformationManager::reservedGas += UnitTypes::Protoss_Pylon.gasPrice();
					InformationManager::reservedMinerals += UnitTypes::Protoss_Pylon.mineralPrice();
					b->unit->build(UnitTypes::Protoss_Pylon, Broodwar->getBuildLocation(UnitTypes::Protoss_Pylon, BWTA::getStartLocation(Broodwar->self())->getTilePosition()));
					pylonIsInProgress = true;
					b->state = 4;
				}
				else if (InformationManager::ourRace == Races::Protoss && pylonIsInProgress){
					b->state = 5;  // SPECIAL CASE FOR PROTOSS
				}
			}
			else{
				b->state = 1;
			}
			break;
		case 1:	// Wait for it......
			if (!pylonIsInProgress  && Broodwar->self()->incompleteUnitCount(InformationManager::ourRace.getSupplyProvider()) < 1){
				// Register an event that draws the target build location
				Broodwar->registerEvent([tile, unitype](Game*)
				{
					Broodwar->drawBoxMap(Position(tile),
						Position(tile + unitype.tileSize()),
						Colors::Green);
				},
					nullptr,					 // condition
					unitype.buildTime() + 100);  // frames to run


				b->unit->move((Position)tile);
				Broodwar << "worker " << std::to_string(b->unit->getID()) << " is going to build " << unitype.toString() << std::endl;
				b->state = 2;
			}

			break;
		case 2:	// Moving to build 
			if (b->unit->getDistance((Position)b->buildTarget) < Broodwar->self()->getRace().getWorker().sightRange()-20){
				b->unit->build(b->buildingProject, b->buildTarget);
				b->state = 3;
			}
			else if (b->unit->isIdle()){
				b->unit->move((Position)b->buildTarget);
			}
			
			break;
		case 3:	// Building

			if (b->unit->isIdle()){
				if (Broodwar->self()->incompleteUnitCount(b->buildingProject) < 1){
					Debug::errorLogMessages("Dave didn't do his job");
					//InformationManager::reservedMinerals -= b->buildingProject.mineralPrice();
					//InformationManager::reservedGas -= b->buildingProject.gasPrice();
					b->buildTarget = getBuildTile(b->buildingProject, b->unit->getTilePosition());
					b->unit->move((Position)b->buildTarget);
					b->state = 2;
				}
				else{
					releaseBuilder(b);
				}
				
			}
			else if (InformationManager::ourRace == Races::Zerg && b->unit->isMorphing()){
				releaseBuilder(b);
			}
			else if (InformationManager::ourRace == Races::Zerg && !b->unit->exists()){
				releaseBuilder(b);
			}
			else if (Broodwar->self()->incompleteUnitCount(b->buildingProject) == 1 && InformationManager::ourRace == Races::Terran){
				b->state = 6;
			}
			break;
		case 4:
			if (b->unit->isIdle()){
				if (Broodwar->self()->incompleteUnitCount(UnitTypes::Protoss_Pylon) < 1){
					pylonIsInProgress = false;
					b->buildTarget = getBuildTile(b->buildingProject, Broodwar->self()->getStartLocation());
					b->state = 0;
				}
			}
		case 5: // Protoss waiting state
			if (Broodwar->self()->incompleteUnitCount(UnitTypes::Protoss_Pylon) < 1){
				b->buildTarget = getBuildTile(b->buildingProject, Broodwar->self()->getStartLocation());
				b->state = 0;
			}
		case 6: // Terran building state
			if (b->unit->isIdle() || b->unit->isGatheringGas()){
				releaseBuilder(b);
			}
		}

	}

}


// Returns a suitable TilePosition to build a given building type near
// specified TilePosition aroundTile, or null if not found. (builder parameter is our worker)
TilePosition BuildingPlacer::getBuildTile(UnitType buildingType, TilePosition aroundTile) {
	TilePosition ret = TilePositions::None;
	int maxDist = 1;
	int stopDist = 40;
	
	// Refinery, Assimilator, Extractor
	if (buildingType.isRefinery()) {
		return Broodwar->getBuildLocation(InformationManager::ourRace.getRefinery(), InformationManager::firstCenter->getTilePosition());
		//for (const auto& geyser : BWTA::getStartLocation(Broodwar->self())->getGeysers()) {
		//	TilePosition p1 = geyser->getInitialTilePosition();
		//	if (geyser->exists()){
		//		return p1;
		//	}
		//}
		//Broodwar << "couldnt find a geyser" << std::endl;
	}
	if (buildingType.isResourceDepot()){
		return expantion();
	}

	while ((maxDist < stopDist) && (ret == TilePositions::None)) {
		for (int i = aroundTile.x - maxDist; i <= aroundTile.x + maxDist; i++) {
			for (int j = aroundTile.y - maxDist; j <= aroundTile.y + maxDist; j++) {
				if (Broodwar->canBuildHere(TilePosition(i, j), buildingType, false) 
					&& (!(InformationManager::ourRace == Races::Terran) || Broodwar->canBuildHere(TilePosition(i+2, j), buildingType, false))) {
					// units that are blocking the tile
					bool unitsInWay = false;
					for (Unit u : Broodwar->getAllUnits()) {
						if ((std::abs(u->getTilePosition().x - i) < 4) && (std::abs(u->getTilePosition().y - j) < 4)) { unitsInWay = true; }
					}
					if (!unitsInWay) {
						return TilePosition(i, j);
					}
					// creep for Zerg
					if (buildingType.requiresCreep()) {
						bool creepMissing = false;
						for (int k = i; k <= i + buildingType.tileWidth(); k++) {
							for (int l = j; l <= j + buildingType.tileHeight(); l++) {
								if (!Broodwar->hasCreep(k, l)) { creepMissing = true; }
								break;
							}
						}
						if (creepMissing) { continue; }
					}
				}
			}
		}
		maxDist ++;
	}

	if (ret == TilePositions::None) { Broodwar << "Unable to find suitable build position for " << buildingType.toString() << std::endl; }
	return ret;
}

TilePosition BuildingPlacer::naturalExpantion(){
	double groundDist = 10000;
	int i = 0;
	int erase_itr = 0;
	TilePosition buildTile = TilePositions::None;
	for (auto &b : InformationManager::baseLocations){
		if (b != InformationManager::mainBase && b->getGroundDistance(InformationManager::mainBase) < groundDist &&
			!b->isIsland() && Broodwar->canBuildHere(b->getTilePosition(), Broodwar->self()->getRace().getCenter())){
			groundDist = b->getGroundDistance(InformationManager::mainBase);
			buildTile = b->getTilePosition();
			erase_itr = i;
		}
		i++;
	}
	if (InformationManager::baseLocations.empty()){
		return buildTile;
	}
	else{

		InformationManager::baseLocations.erase(InformationManager::baseLocations.begin() + erase_itr);
		return buildTile;
	}
	
}

TilePosition BuildingPlacer::expantion(){
	double globalMinDist = 10000.0;
	int i = 0;
	int erase_itr = 0;
	BWTA::BaseLocation* ownBase = BWTA::getStartLocation(Broodwar->self());
	TilePosition buildTile = TilePositions::None;
	for (auto b : BWTA::getBaseLocations() ){
		if (b == ownBase){
			continue;
		}
		double groundDist = ownBase->getGroundDistance(b);
		if (Broodwar->canBuildHere(b->getTilePosition(), InformationManager::ourRace.getCenter(), false) && groundDist > 0 && groundDist < globalMinDist){
			globalMinDist = groundDist;
			buildTile = b->getTilePosition();

		}
	}
	return buildTile;
}



//// OLD STUFF
void BuildingPlacer::buildOrTrain(UnitType ut){
	if (ut.isBuilding()){
		bool inProgress = false;
		for (auto &b : InformationManager::orderedBuildings){
			if (ut == b){
				inProgress = true;
			}
		}
		if (!inProgress){
			for (auto myUnit : InformationManager::workerUnits) {

				if (myUnit->unit != IntelManager::scout->unit && !myUnit->unit->isConstructing() && !myUnit->unit->isGatheringGas()) {
					//Find a builtile
					TilePosition buildTile = getBuildTile(ut, Broodwar->self()->getStartLocation());
					//and, if found, send the worker to build it (and leave others alone - break;)
					if (buildTile != TilePositions::None) {
						// Register an event that draws the target build location
						Broodwar->registerEvent([buildTile, ut](Game*)
						{
							Broodwar->drawBoxMap(Position(buildTile),
								Position(buildTile + ut.tileSize()),
								Colors::Green);
						},
							nullptr,  // condition
							ut.buildTime() + 100);  // frames to run
						InformationManager::reservedMinerals += ut.mineralPrice();
						InformationManager::reservedGas += ut.gasPrice();
						myUnit->unit->build(ut, buildTile);
						InformationManager::orderedBuildings.push_back(ut);
						Broodwar << "worker is building " << ut.toString() << std::endl;
						break;
					}
					else{
						//Broodwar << "Expantion BuildTile = none" << std::endl;
						//if (InformationManager::ourRace == Races::Protoss && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Pylon) < 1){
						//	buildOrTrain(UnitTypes::Protoss_Pylon);
						//}
						//break;
					}
				}
			}
		}
	}
	else{
		for (auto &u : Broodwar->self()->getUnits()){//Tp be refactored with more dynamic code
			if (u->canTrain(ut) && u->isIdle()){
				u->train(ut);
				//Broodwar << u->getType().toString() << " is building " << toBuild.toString() << std::endl;
				break;
			}
		}
	}
}