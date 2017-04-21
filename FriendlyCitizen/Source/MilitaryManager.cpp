#include "MilitaryManager.h"
#include "InformationManager.h"
#include "Debug.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

void MilitaryManager::onFrame(){
	std::vector<BWAPI::Unit> military;
	for (auto &u : Broodwar->self()->getUnits()){
		if (!u->getType().isWorker() && u->getType().canAttack() && u->isIdle()){
			military.push_back(u);
			for (EnemyUnit eu : InformationManager::enemyUnits){
				Unit temp = *eu.selfCopy;
				u->attack((BWAPI::Position) temp->getPosition());
				break;
			}
		}	
	}
	if (military.size() > 5){
		for (auto &u : military){
			u->attack(u->getClosestUnit(Filter::IsEnemy)->getPosition());
		}
	}

}

