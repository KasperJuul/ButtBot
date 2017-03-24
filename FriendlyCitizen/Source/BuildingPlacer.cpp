#include "BuildingPlacer.h"
#include "InformationManager.h"
#include "Debug.h"
#include "ResourceManager.h"

using namespace BWAPI;
bool BuildingPlacer::supplyProviderIsBeingBuild = false;

void BuildingPlacer::onStart(){

}

void BuildingPlacer::onFrame(){
	static int lastChecked = 0;

	if (InformationManager::firstCenter->isIdle() && ResourceManager::wrkUnits.size() < 20 &&
		!InformationManager::firstCenter->train(InformationManager::firstCenter->getType().getRace().getWorker())){
		// If that fails, draw the error at the location so that you can visibly see what went wrong!
		// However, drawing the error once will only appear for a single frame
		// so create an event that keeps it on the screen for some frames
		Position pos = InformationManager::firstCenter->getPosition();
		Error lastErr = Broodwar->getLastError();
		Broodwar->registerEvent([pos, lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
			nullptr,    // condition
			Broodwar->getLatencyFrames());  // frames to run
	}

	//if we're running out of supply and have enough minerals ...
	if (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() < 6 && Broodwar->self()->minerals() >= 100 && !supplyProviderIsBeingBuild) {
		//iterate over units to find a worker
		for (ResourceManager::workerUnit &myUnit : ResourceManager::wrkUnits) {
			if (myUnit.status == "Idle" || myUnit.status == "Returning Cargo") {
				//get a nice place to build a supply depot
				UnitType supplyDepot = Broodwar->self()->getRace().getSupplyProvider();
				TilePosition buildTile =
					getBuildTile(myUnit.unit, supplyDepot, InformationManager::firstCenter->getTilePosition());
				//and, if found, send the worker to build it (and leave others alone - break;)
				if (buildTile != TilePositions::None) {
					myUnit.unit->build(supplyDepot, buildTile);
					myUnit.status = "Building";
					Broodwar << "worker is building supplyprovider" << std::endl;
					supplyProviderIsBeingBuild = true;
					break;
				}
			}
		}
	}
}


// Returns a suitable TilePosition to build a given building type near
// specified TilePosition aroundTile, or null if not found. (builder parameter is our worker)
TilePosition BuildingPlacer::getBuildTile(Unit builder, UnitType buildingType, TilePosition aroundTile) {
	TilePosition ret = TilePositions::None;
	int maxDist = 3;
	int stopDist = 40;
	
	// Refinery, Assimilator, Extractor
	if (buildingType.isRefinery()) {
		for (Unit n : Broodwar->neutral()->getUnits()) {
			if ((n->getType() == UnitTypes::Resource_Vespene_Geyser) &&
				(std::abs(n->getTilePosition().x - aroundTile.x) < stopDist) &&
				(std::abs(n->getTilePosition().y - aroundTile.y) < stopDist)
				) return n->getTilePosition();
		}
	}

	while ((maxDist < stopDist) && (ret == TilePositions::None)) {
		for (int i = aroundTile.x - maxDist; i <= aroundTile.x + maxDist; i++) {
			for (int j = aroundTile.y - maxDist; j <= aroundTile.y + maxDist; j++) {
				if (Broodwar->canBuildHere(TilePosition(i, j), buildingType, builder, false)) {
					// units that are blocking the tile
					bool unitsInWay = false;
					for (Unit u : Broodwar->getAllUnits()) {
						if (u->getID() == builder->getID()) { continue; }
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
		maxDist += 2;
	}

	if (ret == TilePositions::None) { Broodwar << "Unable to find suitable build position for " << buildingType.toString() << std::endl; }
	return ret;
}
