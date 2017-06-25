#pragma once
#include "BuildingPlanner.h"
#include "InformationManager.h"
#include <iostream>
#include <math.h>
#include <algorithm>

std::string BuildingPlanner::plan;

SpecialReq BuildingPlanner::enemyStealth = SpecialReq::NotNeeded;
float BuildingPlanner::econ = 0;
float BuildingPlanner::mili = 0;
float BuildingPlanner::tech = 0;
float BuildingPlanner::maxCombat = 1;
float BuildingPlanner::maxCombatEnemy = 1;
float BuildingPlanner::maxCombatTotal = 1;

//Deprecated
void BuildingPlanner::plannerOnFrame(){

	/*makePlan();//TEST, REMOVE LATER
	if (false){ //If there is a plan...
		//Execute plan
	}
	else {//... Else, make a new plan
		BuildingPlanner::makePlan();
	}*/
}

BWAPI::UnitType BuildingPlanner::chooseBetweenMilitary(std::vector<TechNode> selection){//Combat only
	BWAPI::UnitType selected;
	float priority = 0;
	for (auto ut : selection){
		if (/*Broodwar->self()->minerals() < ut.selfType.mineralPrice() ||*/ Broodwar->self()->gas() < ut.selfType.gasPrice()){
			continue;
		}
		float tempPriority = combatValue(ut)/maxCombat * 100 + specialValue(ut,true);//To be made dynamic later with military manager
		//tempPriority += specialValue(ut);
		if (tempPriority > priority){
			priority = tempPriority;
			selected = ut.selfType;
		}
	}
	return selected;
}

std::vector<Priority> BuildingPlanner::order(std::vector<Priority> military, std::vector<Priority> economy, Priority technology){
	std::vector<Priority> finalOrder;
	econ = 0;
	mili = 0;
	tech = 0;
	for (auto u : InformationManager::militaryUnits){//Contains refinery and support. Needs to be fixed.
		if (u->unit->getType() == InformationManager::ourRace.getSupplyProvider()){
			econ += InformationManager::ourRace.getSupplyProvider().mineralPrice();
		}
		else if (u->unit->getType() == InformationManager::ourRace.getRefinery()){
			econ += InformationManager::ourRace.getRefinery().mineralPrice();
		}
		else {
			mili += u->unit->getType().mineralPrice() + u->unit->getType().gasPrice();
		}
	}
	for (auto u : InformationManager::militaryBuildings){
		mili += u->unit->getType().mineralPrice() + u->unit->getType().gasPrice();
	}
	std::set<BWAPI::UnitType> first;
	for (auto u : InformationManager::productionBuildings){//Nexus is here
		if (u->unit->getType() == InformationManager::ourRace.getCenter()){
			econ += InformationManager::ourRace.getCenter().mineralPrice();
			continue;
		}
		int cSize = first.size();
		first.insert(u->unit->getType());
		if (cSize!=first.size()){
			tech += u->unit->getType().mineralPrice() + u->unit->getType().gasPrice();
		}
	}
	for (auto u : InformationManager::workerUnits){
		econ += InformationManager::ourRace.getSupplyProvider().mineralPrice();

	}

	float totalSpend = econ + tech + mili;
	econ -= 1000;
	econ = econ / totalSpend * 100;
	tech = tech / totalSpend * 100;
	mili = mili / totalSpend * 100;

	mili = 50 - mili+(float)0.02;
	econ = 40 - econ+(float)0.01;
	tech = 10 - tech;

	Priority supplier;
	supplier.priority = econValue(InformationManager::ourRace.getSupplyProvider());
	supplier.unitType = InformationManager::ourRace.getSupplyProvider();
	supplier.declaration = TypeDec::UnitDec;
	if (supplier.priority > 50 && Broodwar->self()->supplyTotal() != 400 && Broodwar->self()->incompleteUnitCount(InformationManager::ourRace.getSupplyProvider()) < 1){
		finalOrder.push_back(supplier);
	}
	for (auto u : Broodwar->self()->getUnits()){
		if (u->getType() == BWAPI::UnitTypes::Protoss_Reaver){
			if (!u->isTraining() && u->getScarabCount() < 5){
				Priority scarab;
				scarab.priority = combatValue(BWAPI::UnitTypes::Protoss_Scarab)/maxCombat* 100;
				scarab.unitType = BWAPI::UnitTypes::Protoss_Scarab;
				scarab.declaration = TypeDec::UnitDec;
				finalOrder.push_back(scarab);
			}
		}
		if (u->getType() == BWAPI::UnitTypes::Protoss_Interceptor){
			if (!u->isTraining() && u->getInterceptorCount() < 5){
				Priority interceptor;
				interceptor.priority = combatValue(BWAPI::UnitTypes::Protoss_Interceptor)/maxCombat * 100;
				interceptor.unitType = BWAPI::UnitTypes::Protoss_Interceptor;
				interceptor.declaration = TypeDec::UnitDec;
				finalOrder.push_back(interceptor);
			}
		}
	}
	//Higher value comes before lower value. If tied, mili>econ>tech. This is not ensured by the structure below, but by the += 0.02~ above.
	if (mili>econ && mili>tech){
		//finalOrder = military;
		for (auto mili : military){
			finalOrder.push_back(mili);
		}

		//Second production creator
		if (Broodwar->self()->getRace() == BWAPI::Races::Protoss){
			int count = Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Gateway);
			if (count > 0 && count < 2){
				Priority temp;
				temp.declaration = TypeDec::UnitDec;
				temp.priority = 0;
				temp.unitType = BWAPI::UnitTypes::Protoss_Gateway;
				finalOrder.push_back(temp);
			}
		}
		else if (Broodwar->self()->getRace() == BWAPI::Races::Zerg){
			int count = Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool);
			int count2 = Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hatchery);
			if (count > 0 && count2 < 2){
				Priority temp;
				temp.declaration = TypeDec::UnitDec;
				temp.priority = 0;
				temp.unitType = BWAPI::UnitTypes::Zerg_Hatchery;
				finalOrder.push_back(temp);
			}
		}
		else if(Broodwar->self()->getRace() == BWAPI::Races::Terran){
			int count = Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Barracks);
			if (count > 0 && count < 2){
				Priority temp;
				temp.declaration = TypeDec::UnitDec;
				temp.priority = 0;
				temp.unitType = BWAPI::UnitTypes::Terran_Barracks;
				finalOrder.push_back(temp);
			}
		}

		for (auto test : finalOrder){
			//Debug::errorLogMessages("Military contains... " + test.unitType.getName());
		}
		//Debug::errorLogMessages("End");
		//mili first
		if (econ > tech){
			//Econ second, tech third
			for (auto p : economy ){
				finalOrder.push_back(p);
			}

			for (auto test : finalOrder){
				//Debug::errorLogMessages("econ with contains... " + test.unitType.getName());
			}
			//Debug::errorLogMessages("End1");
			finalOrder.push_back(technology);

			for (auto test : finalOrder){
				//Debug::errorLogMessages("tech with contains... " + test.unitType.getName());
			}
			//Debug::errorLogMessages("End2");
		}
		else {
			//Tech second, econ third
			finalOrder.push_back(technology);
			for (auto p : economy){
				finalOrder.push_back(p);
			}
		}
	}
	else if (econ > mili && econ > tech){
		for (auto p : economy){
			finalOrder.push_back(p);
		}
		//Econ first
		if (mili > tech){
			//Mili second, tech third
			for (auto p : military){
				finalOrder.push_back(p);
			}
			finalOrder.push_back(technology);
		}
		else {
			//Tech second, mili first
			finalOrder.push_back(technology);
			for (auto p : military){
				finalOrder.push_back(p);
			}
		}
	}
	else {
		//Tech first
		finalOrder.push_back(technology);
		if (mili > econ){
			//Mili second, econ third
			for (auto p : military){
				finalOrder.push_back(p);
			}
			for (auto p : economy){
				finalOrder.push_back(p);
			}
		}
		else {
			//Econ second, mili third
			for(auto p : economy){
				finalOrder.push_back(p);
			}
			for (auto p : military){
				finalOrder.push_back(p);
			}
		}
	}

	if (Broodwar->self()->completedUnitCount(InformationManager::ourRace.getCenter()) > 1){
		/*if (Broodwar->self()->completedUnitCount(InformationManager::ourRace.getRefinery()) < Broodwar->self()->completedUnitCount(InformationManager::ourRace.getCenter())){

			Priority temp;
			temp.declaration = TypeDec::UnitDec;
			temp.priority = 0;
			temp.unitType = InformationManager::ourRace.getRefinery();
			finalOrder.push_back(temp);
		}
		else {

			Priority temp;
			temp.declaration = TypeDec::UnitDec;
			temp.priority = 0;
			temp.unitType = InformationManager::ourRace.getCenter();
			finalOrder.push_back(temp);
		}*///For the fix on refineries.
		Priority temp;
		temp.declaration = TypeDec::UnitDec;
		temp.priority = 0;
		temp.unitType = InformationManager::ourRace.getCenter();
		finalOrder.push_back(temp);
	}

	if (Broodwar->self()->minerals() > 1500 && Broodwar->self()->getRace != BWAPI::Races::Zerg){
		Priority temp;
		temp.declaration = TypeDec::UnitDec;
		temp.priority = 0;
		if (Broodwar->self()->getRace() == BWAPI::Races::Protoss){
			temp.unitType = BWAPI::UnitTypes::Protoss_Gateway;
		}
		else {
			temp.unitType = BWAPI::UnitTypes::Terran_Barracks;
		}
		finalOrder.push_back(temp);
	}

	return finalOrder;
}

std::vector<Priority> BuildingPlanner::findOrder(){//CURRENT REFACTOR OF THIS CODE'S MAIN FUNCTION.
	maxCombatTotal = 1;
	maxCombat = 1;
	for (auto ot : InformationManager::ourTech){
		float val = combatValue(ot);
		if (val > maxCombat){
			maxCombat = val;
		}
	}
	maxCombatEnemy = 1;
	for (auto ot : InformationManager::theirTech){
		float val = combatValue(ot);
		if (val > maxCombatEnemy){
			maxCombatEnemy = val;
		}
	}
	if (maxCombatEnemy > 1){
		maxCombatTotal = (maxCombat + maxCombatEnemy) / 2;
		maxCombat = maxCombatTotal;
		maxCombatEnemy = maxCombatTotal;
	}

	std::vector<Priority> economy;
	std::vector<Priority> military;
	Priority technology;
	for (auto u : InformationManager::productionBuildings){
		if (u->unit->isIdle()){
			if (u->unit->getType() == InformationManager::ourRace.getCenter()){
				//It can build a probe, woo.
				Priority temp;
				temp.priority = econValue(InformationManager::ourRace.getWorker());
				temp.unitType = InformationManager::ourRace.getWorker();
				temp.declaration = TypeDec::UnitDec;
				if (InformationManager::workerUnits.size() <= 50){
					economy.push_back(temp);

				}
			}
			else {
				if (Broodwar->self()->getRace() != BWAPI::Races::Zerg){//Zergs produces all their units from larva or drones (or upgrade morphs)! They have NO production buildings!
					for (auto ut : u->unit->getType().buildsWhat()){//This unit builds x...
						std::vector<TechNode> toSelectFrom;
						for (auto ot : InformationManager::ourTech){//Which is represented somewhere in ourTech...
							if (ot.selfType == ut){//Which has now been found.
								bool exists = true;
								for (auto pre : ot.precondition){//But have we unlocked the unit as of yet?
									if (!pre->exists){
										exists = false;
									}
								}
								if (exists){//Yes!
									toSelectFrom.push_back(ot);
									//Debug::errorLogMessages("This shouldn't be called atm");
								}//Or no?
								break;
							}
						}
						if (!toSelectFrom.empty()){
							//Debug::errorLogMessages("This shouldn't be called atm");
							UnitType tempType = chooseBetweenMilitary(toSelectFrom);
							Priority temp;
							temp.priority = combatValue(tempType)/maxCombat*100 + specialValue(tempType,true);
							temp.unitType = tempType;
							temp.declaration = TypeDec::UnitDec;
							military.push_back(temp);
						}
					}
					
				}
				else { //If Zerg...

				}

			}
		}
	}//End of Production buildings

	if (Broodwar->self()->getRace() == BWAPI::Races::Zerg){
		std::vector<TechNode> toSelectFrom;
		for (auto ut : BWAPI::UnitTypes::Zerg_Larva.buildsWhat()){//This unit builds x...
			if (ut == BWAPI::UnitTypes::Zerg_Drone ||ut == BWAPI::UnitTypes::Zerg_Overlord){
				continue;
			}
			for (auto ot : InformationManager::ourTech){//Which is represented somewhere in ourTech...
				if (ot.selfType == ut){//Which has now been found.
					bool exists = true;
					for (auto pre : ot.precondition){//But have we unlocked the unit as of yet?
						if (!pre->exists){
							exists = false;
						}
					}
					if (exists){//Yes!
						toSelectFrom.push_back(ot);
						//Debug::errorLogMessages("This shouldn't be called atm");
					}//Or no?
					break;
				}
			}
		}

		if (!toSelectFrom.empty()){
			//Debug::errorLogMessages("This shouldn't be called atm");
			UnitType tempType = chooseBetweenMilitary(toSelectFrom);
			Priority temp;
			temp.priority = combatValue(tempType) / maxCombat * 100 + specialValue(tempType,true);
			temp.unitType = tempType;
			temp.declaration = TypeDec::UnitDec;
			military.push_back(temp);
			if (tempType == UnitTypes::Zerg_Zergling){
				//Broodwar << "Just another Zerglin' " << std::to_string(toSelectFrom.size()) << std::endl;
				std::vector<std::string> larvaCanProduce;
				larvaCanProduce.push_back(" ");
				for (auto ut : toSelectFrom){
					larvaCanProduce.push_back(ut.selfType.getName() + std::to_string(combatValue(ut)/maxCombat*100+specialValue(ut,true)));
				}
				Debug::writeLog(larvaCanProduce,"selectfrom","test");
			}
		}
	}

	std::vector<int> possibleNodes;
	std::vector<int> currentNodes;//Probably pointless?
	std::vector<int> futureNodes;
	for (int i = 0; i < InformationManager::ourTech.size(); i++){
		if (InformationManager::ourTech.at(i).exists){
			currentNodes.push_back(i);
			continue;
		}
		bool valid = true;
		for (int i2 = 0; i2 < InformationManager::ourTech.at(i).precondition.size(); i2++){
			if (!InformationManager::ourTech.at(i).precondition.at(i2)->exists) {
				valid = false;
				break;
			}
		}
		if (valid){
			possibleNodes.push_back(i);
		}
		else {
			futureNodes.push_back(i);
		}
	}

	float combatMax = 0;
	std::vector<float> combatValuetoID = *new std::vector<float>();
	for (auto ot : InformationManager::ourTech){
		int val = combatValue(ot);
		combatValuetoID.push_back(val);
		if (val > combatMax){
			combatMax = val;
		}
	}

	std::vector<float> specialValuetoID = *new std::vector<float>();
	for (auto ot : InformationManager::ourTech){
		int val = specialValue(ot,true);
		specialValuetoID.push_back(val);
	}

	for (int i = 0; i < combatValuetoID.size(); i++){
		combatValuetoID.at(i) = (combatValuetoID.at(i) / combatMax) * 100 + specialValuetoID.at(i);
	}

	std::vector<float> techValuetoID = techValue(possibleNodes, futureNodes, combatValuetoID);//TODO: Should be remade and made prettier, but works.

	UnitType techSelect = BWAPI::UnitTypes::None;
	float max = 0;
	int i = 0;
	for (auto f : techValuetoID){
		if (f > max){
			techSelect = InformationManager::ourTech.at(i).selfType;
			max = f;
			//Debug::errorLogMessages(techSelect.getName()+ " has been chosen with value " + std::to_string(max));
		}
		i++;
	}

	technology.priority = max;
	technology.unitType = techSelect;
	technology.declaration = TypeDec::UnitDec;

	if (Broodwar->self()->getRace() == BWAPI::Races::Zerg){
		Priority worker;
		worker.priority = econValue(InformationManager::ourRace.getWorker());
		worker.unitType = InformationManager::ourRace.getWorker();
		worker.declaration = TypeDec::UnitDec;
		if (InformationManager::workerUnits.size() <= 50){
			economy.push_back(worker);

		}
	}

	Priority center;
	center.priority = econValue(InformationManager::ourRace.getCenter());
	center.unitType = InformationManager::ourRace.getCenter();
	center.declaration = TypeDec::UnitDec;
	if (center.priority > 50){
		economy.push_back(center);
	}
	


	Priority refinery;
	refinery.priority = econValue(InformationManager::ourRace.getRefinery());
	refinery.unitType = InformationManager::ourRace.getRefinery();
	refinery.declaration = TypeDec::UnitDec;
	if (refinery.priority > 50 &&
		Broodwar->self()->completedUnitCount(InformationManager::ourRace.getRefinery()) < Broodwar->self()->completedUnitCount(InformationManager::ourRace.getCenter())){
		economy.push_back(refinery);
	}


	Priority supplier;
	supplier.priority = econValue(InformationManager::ourRace.getSupplyProvider());
	supplier.unitType = InformationManager::ourRace.getSupplyProvider();
	supplier.declaration = TypeDec::UnitDec;
	if (supplier.priority > 50){
		//economy.push_back(supplier);
	}
	
	

	std::sort(military.rbegin(),military.rend());
	std::sort(economy.rbegin(), economy.rend());
	return order(military, economy, technology);
}

float BuildingPlanner::combatValue(TechNode toAnalyze){
	return combatValue(toAnalyze.selfType);
}

float BuildingPlanner::combatValue(UnitType toAnalyze){
	//Manually handled edge cases:
	if (toAnalyze == BWAPI::UnitTypes::Protoss_Scarab){//Due to their insta-damage ability, the calculations for DPF bug out, causing scarabs to get overrated
		return 50;
	}
	if (toAnalyze == BWAPI::UnitTypes::Protoss_Reaver){//Due to the fact that they don't directly attack, the DPF doesn't work, causing them to be underrated
		return 250;
	}
	if (toAnalyze == BWAPI::UnitTypes::Protoss_Interceptor){//Same as Scarab, although not as severe.
		return 25;
	}
	if (toAnalyze == BWAPI::UnitTypes::Protoss_Carrier){//Same as reaver
		return 200;
	}
	if (toAnalyze == BWAPI::UnitTypes::Terran_Barracks){
		return 100;
	}
	if (toAnalyze.groundWeapon() == BWAPI::WeaponTypes::None){
		return 0;
	}
	else {
		float dpf = (float)toAnalyze.groundWeapon().damageAmount() * (float)toAnalyze.maxGroundHits() / (float)toAnalyze.groundWeapon().damageCooldown(); //Damage per frame
		float type = 1;
		if (toAnalyze.groundWeapon().damageType() == BWAPI::DamageTypes::Concussive){
			type -= 0.25;
		}
		if (toAnalyze.groundWeapon().damageType() == BWAPI::DamageTypes::Explosive){
			type -= 0.25;
		}
		if (toAnalyze.groundWeapon().outerSplashRadius() > 0){
			type += 0.25;
		}

		//Debug::errorLogMessages("The unit " + toAnalyze.getName() + " has the following formula: " + std::to_string(dpf) + " * " + std::to_string(type) + " * (" + std::to_string(toAnalyze.maxHitPoints()) + "+" + std::to_string(toAnalyze.maxShields()) + ")*(1+0.1*" + std::to_string(toAnalyze.selfType.armor()) + ")");
		return dpf*type*(toAnalyze.maxHitPoints() + toAnalyze.maxShields())*(1 + 0.1*toAnalyze.armor())+pow(toAnalyze.groundWeapon().maxRange()/32,2)+pow(toAnalyze.airWeapon().maxRange()/32,2);
	}
}

float BuildingPlanner::econValue(TechNode toAnalyze){
	return econValue(toAnalyze.selfType);
}
	
float BuildingPlanner::econValue(UnitType toAnalyze){
	if (Broodwar->self()->getRace().getSupplyProvider().getName() == toAnalyze.getName()){//Support unit
		if ((float)Broodwar->self()->supplyUsed()*1.25 > Broodwar->self()->supplyTotal()){//Supporter, ie: Pylons, overlords, etc.
			return 100;
		}
		else {
			return 0;
		}
	}
	if (Broodwar->self()->getRace().getRefinery().getName() == toAnalyze.getName()){//Gas production
		int allyGas = 0;
		int count = Broodwar->self()->completedUnitCount(Broodwar->self()->getRace().getRefinery());
		for (auto r : InformationManager::regions){
			if (r.owner == OwningPlayer::Self){
				allyGas++;
			}
		}
		if (allyGas <= count){
			return 0;
		}

		float F = 100; //Predetermined factor
		float toDivide = 11.0 + (21 * count) - (float)InformationManager::workerUnits.size();
		if (toDivide <= 1){ toDivide = 1; }
		return F / toDivide; //Formula: F / (Minerals in regions we own - probes we own + 0.1). +0.1 to avoid division by zero.
	}
	if (Broodwar->self()->getRace().getWorker().getName() == toAnalyze.getName()){//Worker
		int count = Broodwar->self()->completedUnitCount(Broodwar->self()->getRace().getCenter());//BUG: Takes the number of all completed, not currently existing
		int count2 = InformationManager::workerUnits.size();
		if (count2 > 21*count ||count2>50){
			return 0;
		}
		float F = 100; //Predetermined factor
		return std::min((float)pow(2 * 9 * count + 3 * count - count2, 2.0), (float)90.0); //Formula: (2*minerals we own + 3 * regions we own with gas) / (probes we own +0.1). +0.1 to avoid division by zero.
	}
	if (Broodwar->self()->getRace().getCenter().getName() == toAnalyze.getName()){//Expansion
		int count = Broodwar->self()->completedUnitCount(Broodwar->self()->getRace().getCenter());
		float F = 100; //Predetermined factor
		float toDivide = (2 * 9 * count + 3 * count - (float)InformationManager::workerUnits.size());
		if (toDivide <= 1){ toDivide = 1; }
		return F / toDivide; //Formula: F / (2*minerals we own + 3 - probes we own + 0.1). +0.1 to avoid division by zero
	}
	//If anything has economic value, it should be included here later.
	return 0;
}

float BuildingPlanner::specialValue(TechNode toAnalyze, bool ally){//TODO: Implement Not Needed and Needed values.
	return specialValue(toAnalyze.selfType, ally);
}

float BuildingPlanner::specialValue(UnitType toAnalyze, bool ally){//TODO: Implement Not Needed and Needed values.
	float val = 0;
	if (ally){
		if (toAnalyze.isBuilding()){//Counts as defensive/static
			if (toAnalyze.isDetector()){//Defensive detector
				val += 10; //Satiated value.
			}
			if (toAnalyze.canAttack() || toAnalyze == BWAPI::UnitTypes::Terran_Bunker){//Defensive turret
				val += 10; //Satiated value.
			}
		}
		else {
			if (toAnalyze.isCloakable() || toAnalyze.hasPermanentCloak()){//Stealth unit
				val += 25;
			}
			if (toAnalyze.isDetector()){
				if (enemyStealth == SpecialReq::NotNeeded){
					val += 10;
				}
				else if (enemyStealth == SpecialReq::Needed){
					val += 500;
				}
				else {
					val += 25;
				}
			}
			if (toAnalyze.groundWeapon() != BWAPI::WeaponTypes::None){//If can attack ground!
				if (toAnalyze.groundWeapon().maxRange() / 32 >= 6){
					val += 25;
				}

				if (toAnalyze.groundWeapon().maxRange() / 32 >= 4 && toAnalyze.topSpeed() > 5){//Temporary functionality - Should be relative to enemy kiteability
					val += 10;
				}

				if (toAnalyze.isFlyer()){//Air VS Ground
					val += 10;
					if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){//Air VS Air
						val += 0;
					}
				}
				else {//Ground unit
					if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){//Ground VS Air
						val += 10;
					}
				}
			}
			else if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){
				if (toAnalyze.isFlyer()){// Air VS Air
					val += 0;
				}
				else { //Ground VS Air
					val += 10;
				}
			}
		}
	}
	else {
		if (toAnalyze.isBuilding()){//Counts as defensive/static
			if (toAnalyze.isDetector()){//Defensive detector
				val += 10; //Satiated value.
			}
			if (toAnalyze.canAttack() || toAnalyze == BWAPI::UnitTypes::Terran_Bunker){//Defensive turret
				val += 10; //Satiated value.
			}
		}
		else {
			if (toAnalyze.isCloakable() || toAnalyze.hasPermanentCloak()){//Stealth unit
				if (enemyStealth == SpecialReq::NotNeeded){//Doesn't make sense, but I'll keep it just to make sure)
					val += 10;
				}
				else if (enemyStealth == SpecialReq::Needed){
					val += 500;
				}
				else {
					val += 25;
				}
			}
			if (toAnalyze.isDetector()){
				
				val += 25;
			}
			if (toAnalyze.groundWeapon() != BWAPI::WeaponTypes::None){//If can attack ground!
				if (toAnalyze.groundWeapon().maxRange() / 32 >= 6){
					val += 25;
				}

				if (toAnalyze.groundWeapon().maxRange() / 32 >= 4 && toAnalyze.topSpeed() > 5){//Temporary functionality - Should be relative to enemy kiteability
					val += 10;
				}

				if (toAnalyze.isFlyer()){//Air VS Ground
					val += 10;
					if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){//Air VS Air
						val += 0;
					}
				}
				else {//Ground unit
					if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){//Ground VS Air
						val += 10;
					}
				}
			}
			else if (toAnalyze.airWeapon() != BWAPI::WeaponTypes::None){
				if (toAnalyze.isFlyer()){// Air VS Air
					val += 0;
				}
				else { //Ground VS Air
					val += 10;
				}
			}
		}
	}
	return val;
}

std::vector<float> BuildingPlanner::techValue(std::vector<int> possibleNodes, std::vector<int> futureNodes, std::vector<float> totalValue){
	std::vector<float> result;
	if (futureNodes.empty()){
		//Teching no longer has any meaning.
		return result;
	}
	else {
		float val = 0;
		for (int i = 0; i < InformationManager::ourTech.size(); i++){//For every node in our tech tree...
			val = 0;
			for (auto i2 : possibleNodes){//If the node is one of the frontier nodes...
				if (i2 == i){
					for (auto i3 : futureNodes){//Find out what nodes this node will unlock...
						for (auto t3 : InformationManager::ourTech.at(i3).precondition){
							if (t3->selfType.getName() == InformationManager::ourTech.at(i).selfType.getName()){
								val += totalValue.at(i3);//And add its value.
								if (totalValue.at(i3) == 0){
									val += 20;
								}
							}
						}
					}
				}
			}
			result.push_back(val);
		}
		return result;
	}
}

//Deprecated but used for debugging purposes
std::vector<Priority> BuildingPlanner::makePlanN(){
	std::vector<Priority> fullList = *new std::vector<Priority>();
	//Units and buildings
	std::vector<int> possibleNodes;
	std::vector<int> currentNodes;//Probably pointless?
	std::vector<int> futureNodes;
	for (int i = 0; i < InformationManager::ourTech.size(); i++){
		if (InformationManager::ourTech.at(i).exists){
			currentNodes.push_back(i);
			continue;
		}
		bool valid = true;
		for (int i2 = 0; i2 < InformationManager::ourTech.at(i).precondition.size(); i2++){
			if (!InformationManager::ourTech.at(i).precondition.at(i2)->exists) {
				valid = false;
				break;
			}
		}
		if (valid){
			possibleNodes.push_back(i);
		}
		else {
			futureNodes.push_back(i);
		}
	}

	float combatMax = 0;
	std::vector<float> combatValuetoID = *new std::vector<float>();
	std::vector<std::string> debug = *new std::vector<std::string>();
	for (auto ot : InformationManager::ourTech){
		int val = combatValue(ot);
		combatValuetoID.push_back(val);
		if (val > combatMax){
			combatMax = val;
		}
		debug.push_back("The unit: " + ot.selfType.getName() + " was given value [" + std::to_string(val) + "]");
	}

	Debug::writeLog(debug,"combat_values", "temporary");

	float econMax = 0;

	std::vector<float> econValuetoID = *new std::vector<float>();
	debug = *new std::vector<std::string>();
	for (auto ot : InformationManager::ourTech){
		int val = econValue(ot);
		econValuetoID.push_back(val);
		if (val > econMax){
			econMax = val;
		}
		debug.push_back("The unit: " + ot.selfType.getName() + " was given value [" + std::to_string(val) + "]");
	}

	Debug::writeLog(debug, "econ_values", "temporary");

	std::vector<float> specialValuetoID = *new std::vector<float>();
	debug = *new std::vector<std::string>();
	for (auto ot : InformationManager::ourTech){
		int val = specialValue(ot, true);
		specialValuetoID.push_back(val);
		debug.push_back("The unit: " + ot.selfType.getName() + " was given value [" + std::to_string(val) + "]");
	}

	Debug::writeLog(debug, "special_values", "temporary");

	debug = *new std::vector<std::string>();
	for (int i = 0; i < combatValuetoID.size(); i++){
		combatValuetoID.at(i) = (combatValuetoID.at(i) / combatMax) * 100 + specialValuetoID.at(i);
		debug.push_back("ID: " + std::to_string(i) + " was assigned the value " + std::to_string(combatValuetoID.at(i) ) );
	}

	Debug::writeLog(debug, "refined_combat_values", "temporary");


	for (int i = 0; i < econValuetoID.size(); i++){
		econValuetoID.at(i) = (econValuetoID.at(i) / econMax) * 100;
	}

	std::vector<float> techValuetoID = techValue(possibleNodes,futureNodes,combatValuetoID);

	debug = *new std::vector<std::string>();
	for (auto f : techValuetoID){
		debug.push_back("Value: " + std::to_string(f));
	}

	Debug::writeLog(debug, "tech_value", "temporary");

	/*std::vector<int> possibleNodesHeuristics;

	for (int i = 0; i < possibleNodes.size(); i++){
		if (possibleNodes.size() == 0){
			break;
		}
		possibleNodesHeuristics.push_back(heuristic(possibleNodes.at(i)));
	}

	for (int i = 0; i < possibleNodesHeuristics.size(); i++){
		Priority temp;
		temp.type = CustomType::UnitDec;
		temp.priority = possibleNodesHeuristics.at(i);
		//temp.type = "UnitType";
		//temp.typeName = possibleNodes.at(i).selfType.getName();
		temp.unitType = possibleNodes.at(i).selfType;
		fullList.push_back(temp);
	}*/

	//Broodwar << possibleNodes.at(chosenID).selfType.getName() << " : " << std::to_string(chosenHeuristicsValue).c_str() << std::endl;
	
	//Tech - To be implemented

	//Upgrades - To be implemented

	//Prioritize Military, Tech and Econ
	int Mil = 0;
	int Tech = 0;
	int Econ = 0;




	//Sort fullList
	std::sort(fullList.begin(), fullList.end());

	std::string nodes = "Ordered priorities \n";
	for (auto p : fullList){
		nodes += p.unitType.getName() + " " + std::to_string(p.priority) + "\n";
	}
	Debug::writeLog(nodes, "heuristics2", "Logs");
	

	
	return fullList;
}

/*UnitType BuildingPlanner::makePlan(){
	std::vector<TechNode> possibleNodes;
	for (int i = 0; i < InformationManager::ourTech.size(); i++){
		bool valid = true;
		for (int i2 = 0; i2 < InformationManager::ourTech.at(i).precondition.size(); i2++){
			if (!InformationManager::ourTech.at(i).precondition.at(i2)->exists) {
				valid = false;
				break;
			}
		}
		if (valid){
			possibleNodes.push_back(InformationManager::ourTech.at(i));
		}
	}

	std::vector<int> possibleNodesHeuristics;

	for (int i = 0; i < possibleNodes.size(); i++){
		if (possibleNodes.size() == 0){
			break;
		}
		possibleNodesHeuristics.push_back(heuristic(possibleNodes.at(i)));
	}

	int chosenID = 0;
	int chosenHeuristicsValue = 0;
	for (int i = 0; i < possibleNodesHeuristics.size(); i++){
		if (chosenHeuristicsValue > possibleNodesHeuristics.at(i)){
			chosenHeuristicsValue = possibleNodesHeuristics.at(i);
			chosenID = i;
		}
	}
	
	//TODO: Handle unresearched technologies.
	//TODO: Order chosenID's type to be constructed.
	plan = possibleNodes.at(chosenID).selfType.getName() +  " : " + std::to_string(chosenHeuristicsValue);

	std::string nodes = "Nodes: \n";
	for (int i = 0; i < possibleNodes.size(); i++){
		nodes += possibleNodes.at(i).selfType.toString() + " [" + std::to_string(possibleNodesHeuristics.at(i)) + "] \n";
	}
	Debug::writeLog(nodes, "heuristics", "Logs");
	//Broodwar << possibleNodes.at(chosenID).selfType.getName() << " : " << std::to_string(chosenHeuristicsValue).c_str() << std::endl;
	
	return UnitType(possibleNodes.at(chosenID).selfType);
}*/

/*int BuildingPlanner::heuristic(TechNode node){//Temporary function to calculate simple heuristical values.
	//TODO: Handle unresearched technologies.
	//TODO: Refactor/clean up code.
	int value = 0;
	//Plan:
	//Saturation heuristics: Too many of the same type of unit is bad.
	int saturation = 0;
	for (auto us : InformationManager::ourUnits){
		if (us.self->getType() == node.selfType) saturation += 10;
		if (us.self->getType() == node.selfType && node.selfType == InformationManager::ourRace.getRefinery()) saturation += 200;
	}
	if (InformationManager::ourRace.getWorker() == node.selfType) saturation -= 100;//We want SOME workers!
	//Balance heuristics: Too many units of one of the three catagories is probably bad.
	int balanceMilitary = 0;
	int balanceEconomy = 0;
	int balanceTech = 0;
	for (auto us : InformationManager::ourUnits){
		if (us.self->getType() == InformationManager::ourRace.getRefinery()){
			balanceEconomy += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
		else if (us.self->getType() == InformationManager::ourRace.getSupplyProvider()){
			balanceEconomy += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
		else if (us.self->getType() == InformationManager::ourRace.getWorker()){
			balanceEconomy += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
		else if (us.self->getType() == InformationManager::ourRace.getCenter()){
			balanceEconomy += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
		else if (us.self->canAttack() && us.self->getType() != InformationManager::ourRace.getWorker()){
			balanceMilitary += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
		else {
			balanceTech += us.self->getType().mineralPrice() + us.self->getType().gasPrice();
		}
	}
	balanceEconomy -= 1000; //Early-game economy

	int balanceValue = 0;
	if (node.selfType == InformationManager::ourRace.getRefinery()){//Economy
		balanceValue += (int)(balanceEconomy*0.3 - balanceMilitary*0.5 - balanceTech*0.2);
	}
	else if (node.selfType == InformationManager::ourRace.getSupplyProvider()){//Economy
		balanceValue += (int)(balanceEconomy * 0.3 - balanceMilitary * 0.2 - balanceTech * 0.5);
	}
	else if (node.selfType == InformationManager::ourRace.getWorker()){//Economy
		balanceValue += (int)(balanceEconomy * 0.3 - balanceMilitary * 0.2 - balanceTech * 0.5);
	}
	else if (node.selfType == InformationManager::ourRace.getCenter()){//Economy
		balanceValue += (int)(balanceEconomy * 0.3 - balanceMilitary * 0.2 - balanceTech * 0.5);
	}
	else if (node.selfType.canAttack()){//Military
		balanceValue += (int)(0 - balanceEconomy * 0.3 + balanceMilitary * 0.2 - balanceTech * 0.5);
		balanceValue -= node.selfType.groundWeapon().damageAmount()*3;
	}
	else {//Tech
		balanceValue += (int)(0 - balanceEconomy * 0.3 - balanceMilitary * 0.2 + balanceTech * 0.5);
		//Tech satiation: Tech buildings that already exists are pointless.
		for (auto ut : InformationManager::ourUnitTypes){
			if (ut == node.selfType){
				balanceValue += 1000;
			}
		}
	}

	//Price heuristics: Relative expensive things is bad. Relative expensiveness relies on income.
	int price = node.selfType.gasPrice() + node.selfType.mineralPrice();
	int incomeAssume = 0;
	for (auto us : InformationManager::ourUnits){
		if (us.self->getType() == InformationManager::ourRace.getWorker()){
			incomeAssume += 10;
		}
	}
	if (incomeAssume == 0) incomeAssume = 1;
	int relativePrice = (int)(price / incomeAssume);

	//Supply and demand heuristics: The closer to the supply-limit we are, the more we want supply buildings.
	int supplyMargin = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed();
	int demandValue = 0;
	if (node.selfType == InformationManager::ourRace.getSupplyProvider()){
		demandValue -= (5 - supplyMargin) * 10;
	}
	else {
		demandValue += (5 - supplyMargin) * 10;
	}

	//Enemy analysis: A whole range of heuristics not to be made as of yet about enemy analysis heuristical results.
	//State heuristics: At some point in time, a state might ask for a certain unit. Maybe.
	//Infrastructure demand: Similar to the above, pylons and creep producers might be necessary for future construction.
	value = demandValue + relativePrice + balanceValue + saturation;
	//Broodwar << "Node: " << node.selfType.toString() << "Value: " << std::to_string(value) << std::endl;
	return(value);
}*/