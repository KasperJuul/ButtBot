#include "MilitaryManager.h"
#include "InformationManager.h"
#include "Debug.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

void MilitaryManager::onFrame(){
	std::vector<BWAPI::Unit> military;
	for (auto &u : Broodwar->self()->getUnits()){
		if (u->canAttack() && !u->getType().isWorker() && !u->isAttacking() && !u->isMoving() && u->isCompleted()){
			for (EnemyUnit eu : InformationManager::enemyUnits){
				Unit temp = *eu.selfCopy;
				u->attack((BWAPI::Position) temp->getPosition());
				break;
			}
		}
	}

}

