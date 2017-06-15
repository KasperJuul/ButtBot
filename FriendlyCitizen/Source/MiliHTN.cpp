#include "MiliHTN.h"

void MiliHTN::invade(MilitaryUnit invader, std::set<BWAPI::Unit> targets, std::vector<EnemyUnit> targetBuildings){
	if (invader.unit->isAttackFrame() || invader.unit->isStartingAttack()){//Inspired by Ualbertabot's smart attack
		return;
	}

	BWAPI::Unit nearest;
	double distance = DBL_MAX;
	bool targetExists = false;;
	for (auto u : targets){
		if (u->getPosition().getDistance(invader.unit->getPosition()) < distance){
			distance = u->getPosition().getDistance(invader.unit->getPosition());
			nearest = u;
			targetExists = true;
		}
	}

	if (!targetExists){
		//Move towards known building location.
		for (auto eu : targetBuildings){
			invader.unit->move(eu.lastSeen);
			break;
		}
	}
	else {

		MiliHTN::attack(invader.unit, nearest);
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
	for (auto u : targets){
		if (u->getPosition().getDistance(defender.unit->getPosition()) < distance){
			distance = u->getPosition().getDistance(defender.unit->getPosition());
			nearest = u;
			targetExists = true;
		}
	}

	if (targetExists){
		
			MiliHTN::attack(defender.unit,nearest);
		
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

//Medium abstraction
BWAPI::Unit chooseTarget(BWAPI::Unit attacker, std::vector<BWAPI::Unit> targets){
	//Make heuristics!
}

//Low abstraction
bool MiliHTN::attack(BWAPI::Unit attacker, BWAPI::Unit target){
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
		if (!attacker->attack(target)){
		}
		else {
			//Broodwar << "LOOOOOOL" << std::endl;
		}
	}
	else {
		
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