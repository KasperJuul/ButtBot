#pragma once
#include "BuildingPlacer.h"
#include "BuildingPlanner.h"
#include "InformationManager.h"
#include "IntelManager.h"
#include "Debug.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace BWTA;

bool BuildingPlacer::supplyProviderIsBeingBuild = false; 
bool BuildingPlacer::xpandIsBeingBuild = false;


void BuildingPlacer::onStart(){
	
}

void BuildingPlacer::onFrame(){
	static int lastChecked = 0;
	bool wait = false;

	UnitType toBuild = BuildingPlanner::makePlan();
	if (InformationManager::reservedMinerals < 0){
		InformationManager::reservedMinerals = 0;
	}


	if (toBuild.mineralPrice() <= Broodwar->self()->minerals() && toBuild.gasPrice() <= Broodwar->self()->gas()){
		if (toBuild.isBuilding()){
			bool inProgress = false;
			for (auto &b : InformationManager::orderedBuildings){
				if (toBuild == b){
					inProgress = true;
				}
			}
			if (!inProgress){
				for (auto myUnit : InformationManager::workerUnits) {

					if (myUnit->unit != IntelManager::scout.self && !myUnit->unit->isConstructing()) {
						//Find a builtile
						TilePosition buildTile = getBuildTile(toBuild, Broodwar->self()->getStartLocation());
						//and, if found, send the worker to build it (and leave others alone - break;)
						if (buildTile != TilePositions::None) {
							// Register an event that draws the target build location
							Broodwar->registerEvent([buildTile, toBuild](Game*)
							{
								Broodwar->drawBoxMap(Position(buildTile),
									Position(buildTile + toBuild.tileSize()),
									Colors::Green);
							},
								nullptr,  // condition
								toBuild.buildTime() + 100);  // frames to run
							InformationManager::reservedMinerals += toBuild.mineralPrice();
							InformationManager::reservedGas += toBuild.gasPrice();
							myUnit->unit->build(toBuild, buildTile);
							InformationManager::orderedBuildings.push_back(toBuild);
							//myUnit->state = UnitState::BUILDING;
							Broodwar << "worker is building " << toBuild.toString() << std::endl;
							break;
						}
						else{
							Broodwar << "Expantion BuildTile = none" << std::endl;
							break;
						}
					}
				}
			}
		}
		else{
			for (auto &u : Broodwar->self()->getUnits()){//Tp be refactored with more dynamic code
				if (u->canTrain(toBuild) && u->isIdle()){
					u->train(toBuild);
					//Broodwar << u->getType().toString() << " is building " << toBuild.toString() << std::endl;
					break;
				}
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
		for (const auto& geyser : BWTA::getStartLocation(Broodwar->self())->getGeysers()) {
			TilePosition p1 = geyser->getInitialTilePosition();
			if (geyser->exists()){
				return p1;
			}
		}
		Broodwar << "couldnt find a geyser" << std::endl;
	}
	if (buildingType.isResourceDepot()){
		return naturalExpantion();
	}


	while ((maxDist < stopDist) && (ret == TilePositions::None)) {
		for (int i = aroundTile.x - maxDist; i <= aroundTile.x + maxDist; i++) {
			for (int j = aroundTile.y - maxDist; j <= aroundTile.y + maxDist; j++) {
				if (Broodwar->canBuildHere(TilePosition(i, j), buildingType, false)) {
					// units that are blocking the tile
					bool unitsInWay = false;
					for (Unit u : Broodwar->getAllUnits()) {
						//if (u->getID() == builder->getID()) { continue; }
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
	InformationManager::baseLocations.erase(InformationManager::baseLocations.begin() + erase_itr);
	return buildTile;
}

void BuildingPlacer::createType(BWAPI::UnitType unit){
	if (unit.isBuilding()){
		Unit builder;
		TilePosition buildtile = getBuildTile(unit, Broodwar->self()->getStartLocation());
		double dist = 10000;
		for (auto &c : InformationManager::centers){
			for (auto &u : c.wrkUnits){
				if (BWTA::getGroundDistance(buildtile,u.unit->getTilePosition()) > dist){
					builder = u.unit;
					break;
				}
			}
		}
		//TilePosition buildtile = getBuildTile(unit, Broodwar->self()->getStartLocation());
		if (builder){
			builder->build(unit, buildtile);
		}
	}
	else{
		Unit trainer;
		for (auto &u : InformationManager::ourUnits){
			if (u.self->canTrain(unit)){
				trainer = u.self;
				break;
			}
		}
		if (trainer){
			trainer->train(unit);
		}
	}

}