#include "BuildingPlacer.h"
#include "InformationManager.h"
#include "Debug.h"

using namespace BWAPI;

int maxDist = 3;

TilePosition BuildingPlacer::getBuildTile(Unit worker, UnitType building){
	TilePosition aroundTile = (TilePosition) InformationManager::firstCenter->getPosition();
	TilePosition ret = TilePositions::None;
	int stopDist = 40;
	

	// Refinery, Assimilator, Extractor
	if (building.isRefinery()) {
		for (Unit n : Broodwar->neutral()->getUnits()) {
			if ((n->getType() == UnitTypes::Resource_Vespene_Geyser) &&
				(abs(n->getTilePosition().x - aroundTile.x) < stopDist) &&
				(abs(n->getTilePosition().y - aroundTile.y) < stopDist)
				) return n->getTilePosition();
		}
	}

	while ((maxDist < stopDist) && (ret == TilePositions::None)) {
		for (int i = aroundTile.x - maxDist; i <= aroundTile.x + maxDist; i++) {
			for (int j = aroundTile.y - maxDist; j <= aroundTile.y + maxDist; j++) {
				if (Broodwar->canBuildHere(TilePosition(i, j), building, worker, false)) {
					// units that are blocking the tile
					bool unitsInWay = false;
					for (Unit u : Broodwar->getAllUnits()) {
						if (u->getID() == worker->getID()){
							continue;
						}
						if ((std::abs(u->getTilePosition().x - i) < 4) && (std::abs(u->getTilePosition().y - j) < 4)){
							unitsInWay = true;
						}
					}
					if (!unitsInWay) {
						return TilePosition(i, j);
					}
				}
			}
		}
		maxDist += 2;
	}

	if (ret == TilePositions::None){
		BWAPI::Broodwar << "Unable to find suitable build position for " << building.toString() << std::endl;
	}
	return ret;
}
