#include "MiliHTN.h"

void MiliHTN::kiteAttack(BWAPI::Unit kiter, BWAPI::Unit target){
	if (!attack(kiter,target)){
	/*BWAPI::UnitCommand currentCommand(kiter->getLastCommand());
	Broodwar << std::to_string(kiter->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount()) <<
	//std::to_string(currentCommand.getType() != BWAPI::UnitCommandTypes::Attack_Unit) <<
	//std::to_string(currentCommand.getTarget() != target) <<
	std::to_string(!kiter->isAttackFrame()) <<
	std::to_string(!kiter->isStartingAttack()) <<
	std::endl;*/
		if (kiter->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount() &&
			//currentCommand.getType() != BWAPI::UnitCommandTypes::Attack_Unit &&
			//currentCommand.getTarget() != target &&
			!kiter->isAttackFrame() &&
			!kiter->isStartingAttack()){//Inspired by Ualbertabot's smart attack
			
			if (kiter->getGroundWeaponCooldown() > 0 || kiter->getAirWeaponCooldown() > 0){//homemade
				kiter->move( kiter->getPosition() * 2 - target->getPosition()); // Inspired by Ualbertabot
			}
		}
	}
	//move(unit, unit->getPosition() * 2 - target->getPosition()) // Inspired by Ualbertabot
}

void MiliHTN::invade(MilitaryUnit invader, std::set<BWAPI::Unit> targets, std::vector<EnemyUnit> targetBuildings){
	if (invader.unit->isAttackFrame() || invader.unit->isStartingAttack()){//Inspired by Ualbertabot's smart attack
		return;
	}

	BWAPI::Unit nearest;
	double distance = DBL_MAX;
	bool targetExists = false;;
	/*for (auto u : targets){
		if (u->getPosition().getDistance(invader.unit->getPosition()) < distance){
			//distance = u->getPosition().getDistance(invader.unit->getPosition());
			//nearest = u;
			targetExists = true;
		}
	}*/
	nearest = chooseTarget(invader.unit, targets);
	
	if (nearest != NULL){
		targetExists = true;
	}

	if (!targetExists){
		//Move towards known building location.
		for (auto eu : targetBuildings){
			invader.unit->move(eu.lastSeen);
			break;
		}
	}
	else {

		int enemyRange = 0;
		int allyRange = 0;
		if (invader.unit->getType().isFlyer()){
			enemyRange = nearest->getType().airWeapon().maxRange();
		}
		else {
			enemyRange = nearest->getType().groundWeapon().maxRange();
		}

		if (nearest->getType().isFlyer()){
			allyRange = invader.unit->getType().airWeapon().maxRange();
		}
		else {
			allyRange = invader.unit->getType().groundWeapon().maxRange();
		}

		if (nearest->getType().topSpeed() <= invader.unit->getType().topSpeed() && allyRange > enemyRange && !nearest->getType().isBuilding()){
			MiliHTN::kiteAttack(invader.unit, nearest);
		}
		else {
			MiliHTN::attack(invader.unit, nearest);
		}
	}

	if (invader.unit->isIdle()){
		//Broodwar << "faaaaaaaaak" << std::endl;
	}
}

void MiliHTN::defend(MilitaryUnit defender, std::set<BWAPI::Unit> targets){
	if (defender.unit->isAttackFrame() || defender.unit->isStartingAttack() ){//Inspired by Ualbertabot's smart attack
		return;
	}

	//TEMP: Just attack nearest.
	BWAPI::Unit nearest;
	int distance = INT_MAX;
	bool targetExists = false;;
	/*for (auto u : targets){
		if (u->getPosition().getDistance(defender.unit->getPosition()) < distance){
			distance = u->getPosition().getDistance(defender.unit->getPosition());
			nearest = u;
			targetExists = true;
		}
	}*/


	nearest = chooseTarget(defender.unit, targets);
	if (nearest != NULL){
		targetExists = true;
	}

	if (targetExists){
		int enemyRange = 0;
		int allyRange = 0;
		if (defender.unit->getType().isFlyer()){
			enemyRange = nearest->getType().airWeapon().maxRange();
		}
		else {
			enemyRange = nearest->getType().groundWeapon().maxRange();
		}

		if (nearest->getType().isFlyer()){
			allyRange = defender.unit->getType().airWeapon().maxRange();
		}
		else {
			allyRange = defender.unit->getType().groundWeapon().maxRange();
		}

		if (nearest->getType().topSpeed() <= defender.unit->getType().topSpeed() && allyRange > enemyRange && !nearest->getType().isBuilding()){
			MiliHTN::kiteAttack(defender.unit, nearest);
		}
		else {
			MiliHTN::attack(defender.unit, nearest);

		}
		
	}
	else {
		std::set<BWAPI::Unit> targets2;
		for (auto u : Broodwar->getUnitsInRadius(defender.unit->getPosition(), 100, BWAPI::Filter::IsEnemy)){
			if (u->getType() != UnitTypes::Zerg_Larva && u->getType().topSpeed() > defender.unit->getType().topSpeed()){
				targets2.insert(u);
				//Broodwar << "I'm slow! " << std::endl;
			}
		}
		
		BWAPI::Unit nearest = NULL;
		if (!targets2.empty()){
			nearest = chooseTarget(defender.unit,targets2);
		}

		if (nearest != NULL){
			MiliHTN::attack(defender.unit, nearest);
		}
		else {

			int realPlacement = defender.placement;
			if (defender.placement > MilitaryManager::allyRegions.size()){
				realPlacement -= MilitaryManager::allyRegions.size();
				BWAPI::Position pos = InformationManager::regions.at(MilitaryManager::disputedRegions.at(realPlacement)).self->getCenter();
				defender.unit->move(pos);
			}
			else {
				BWAPI::Position pos = InformationManager::regions.at(MilitaryManager::allyRegions.at(realPlacement)).self->getCenter();
				defender.unit->move(pos);

			}
		}
	}
}

//Medium abstraction
BWAPI::Unit MiliHTN::chooseTarget(BWAPI::Unit attacker, std::set<BWAPI::Unit> targets){
	double min = DBL_MAX;
	BWAPI::Unit bestTarget;
	for (auto u : targets){
		if (u->getType().isFlyer() && attacker->getType().airWeapon().damageAmount() < 1 && attacker->getType() != BWAPI::UnitTypes::Protoss_Carrier){
			continue;
		}
		if (!u->getType().isFlyer() && attacker->getType().groundWeapon().damageAmount() < 1 && attacker->getType() != BWAPI::UnitTypes::Protoss_Reaver && attacker->getType() != BWAPI::UnitTypes::Protoss_Carrier){
			continue;
		}
		if (u->isCloaked()){
			bool detector = false;
			for (auto ally : Broodwar->getUnitsInRadius(u->getPosition(),384,BWAPI::Filter::IsAlly)){
				if (!ally->getType().isDetector()){
					detector = true;
					break;
				}
			}
			if (!detector){
				continue;
			}
		}

		double damage = u->getType().groundWeapon().damageAmount() + u->getType().airWeapon().damageAmount();
		if (damage < 1){
			damage = 1;
		}

		//Exception types
		if (u->getType() == BWAPI::UnitTypes::Terran_Barracks){
			damage = 10;
		}
		if (u->getType() == BWAPI::UnitTypes::Protoss_Carrier){
			damage = 10;
		}
		if (u->getType() == BWAPI::UnitTypes::Protoss_Reaver){
			damage = 10;
		}

		double val = u->getDistance(attacker)/damage - damage / (double)(u->getHitPoints() + u->getShields());
		if (val < min){
			min = val;
			bestTarget = u;
		}
	}

	if (min == DBL_MAX){
		return NULL;
	}
	return bestTarget;
}

//Low abstraction
bool MiliHTN::attack(BWAPI::Unit attacker, BWAPI::Unit target){
	/*if (attacker->getType() == BWAPI::UnitTypes::Protoss_Carrier || attacker->getType() == BWAPI::UnitTypes::Protoss_Reaver){
		return attacker->attack(target);
	}*/
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());
	/*Broodwar << std::to_string(attacker->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount()) <<
		//std::to_string(currentCommand.getType() != BWAPI::UnitCommandTypes::Attack_Unit) <<
		std::to_string(currentCommand.getTarget() != target) <<
		std::to_string(!attacker->isAttackFrame()) <<
		std::to_string(!attacker->isStartingAttack()) <<
		std::endl;*/
	if (attacker->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount() &&
		//currentCommand.getType() != BWAPI::UnitCommandTypes::Attack_Unit &&
		currentCommand.getTarget() != target &&
		!attacker->isAttackFrame() &&
		!attacker->isStartingAttack()){//Inspired by Ualbertabot's smart attack
		return attacker->attack(target);
		
	}
	else if (currentCommand.getTarget() != target){
		return attacker->attack(target);
	}

	return false;
}


/*
//Depcrecated
//High abstraction. Can include formations, strategies, etc.

//Medium abstraction. Includes at least some selection between various options for the individual unit.
std::vector<Action> attack(BWAPI::Unit unit, BWAPI::Unit target){
	BWAPI::WeaponType ally;
	BWAPI::WeaponType enemy;
	if (target->getType().isFlyer()){
		ally = unit->getType().airWeapon();
	}
	else {
		ally = unit->getType().groundWeapon();
	}

	if (unit->getType().isFlyer()){
		enemy = target->getType().airWeapon();
	}
	else {
		enemy = target->getType().groundWeapon();
	}

	if (target->getType().topSpeed() <= unit->getType().topSpeed() && ally.maxRange() > enemy.maxRange()){

		Action temp;
		temp.action = ActionDec::Attack;
		PositionOrUnit temp2;
		temp2.isUnit = true;
		temp2.unit = target;
		temp.target = temp2;
		std::vector<Action> temp3;
		temp3.push_back(temp);
		return temp3;
		//return kite(unit,target);//Kite
	}
	else {//Straight attack
		Action temp;
		temp.action = ActionDec::Attack;
		PositionOrUnit temp2;
		temp2.isUnit = true;
		temp2.unit = target;
		temp.target = temp2;
		std::vector<Action> temp3;
		temp3.push_back(temp);
		return temp3;
	}
}

std::vector<Action> kite(BWAPI::Unit unit, BWAPI::Unit target){
	std::vector<Action> tempReturn;
	Action temp;
	if (target->getType().isFlyer()){
		if (unit->getAirWeaponCooldown() == 0){
			temp.action = ActionDec::Attack;
			PositionOrUnit temp2;
			temp2.isUnit = true;
			temp2.unit = target;
			temp.target = temp2;
			tempReturn.push_back(temp);
		}
		else if (!unit->isAttackFrame()){//Section inspired by Ualbertabot
			//Fleeeeee!
//			tempReturn = moveTo(unit, unit->getPosition() * 2 - target->getPosition());
		}
		else {
			temp.action = ActionDec::None;
			tempReturn.push_back(temp);
		}
	}
	else {
		if (unit->getGroundWeaponCooldown() == 0){
			temp.action = ActionDec::Attack;
			PositionOrUnit tempTarget;
			tempTarget.isUnit = true;
			tempTarget.unit = target;
			temp.target = tempTarget;
			tempReturn.push_back(temp);
		}
		else if (!unit->isAttackFrame()){//Section inspired by Ualbertabot
			//Fleeeeee!
//			tempReturn = moveTo(unit,unit->getPosition() * 2 - target->getPosition());
		}
		else {
			temp.action = ActionDec::None;
			tempReturn.push_back(temp);
		}
	}
	return tempReturn;
}

//Low abstraction. Closely related to the base commands.
std::vector<Action> moveTo(BWAPI::Unit unit, BWAPI::Position target){
	Action temp;
	temp.action = ActionDec::Move;
	PositionOrUnit tempTarget;
	tempTarget.isPosition = true;
	tempTarget.position = target;
	temp.target = tempTarget;
	std::vector<Action> tempReturn;
	tempReturn.push_back(temp);
	return tempReturn;
}

std::vector<Action> strictAttack(BWAPI::Unit unit, BWAPI::Unit target){//Disallows kiting. Might be useful for blockading formations?
	Action temp;
	temp.action = ActionDec::Attack;
	PositionOrUnit tempTarget;
	tempTarget.isUnit = true;
	tempTarget.unit = target;
	temp.target = tempTarget;
	std::vector<Action> tempReturn;
	tempReturn.push_back(temp);
	return tempReturn;
}*/