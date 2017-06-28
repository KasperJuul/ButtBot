#pragma once
#include "MilitaryManager.h"
#include "InformationManager.h"
#include "BuildingPlanner.h"
#include "Debug.h"
#include <BWTA.h>
#include <iostream>
#include <vector>

int MilitaryManager::regionCounter = 0;
MainStates MilitaryManager::mainState = MainStates::Defensive;
std::vector<int> MilitaryManager::enemyRegions;
std::vector<int> MilitaryManager::disputedRegions;
std::vector<int> MilitaryManager::allyRegions;

void enemyAnalyze(){
	BuildingPlanner::enemyStealth = SpecialReq::NotNeeded;
	for (auto eu : InformationManager::enemyUnits){
		if (eu.selfType.isCloakable() || eu.selfType.hasPermanentCloak() || eu.selfType.isBurrowable() && eu.selfType != BWAPI::UnitTypes::Terran_Vulture_Spider_Mine){
			bool detector = false;
			for (auto u : Broodwar->self()->getUnits()){
				if (u->getType().isDetector()){
					detector = true;
					break;
				}
			}
			if (detector){
				BuildingPlanner::enemyStealth = SpecialReq::Satiated;
			}
			else if(BuildingPlanner::enemyStealth == SpecialReq::NotNeeded) {//Don't undo satiated
				BuildingPlanner::enemyStealth = SpecialReq::Needed;
			}
		}
	}

	BuildingPlanner::enemyAir = SpecialReq::NotNeeded;
	for (auto eu : InformationManager::enemyUnits){
		if (eu.selfType.isFlyer() && eu.selfType.groundWeapon().damageAmount() > 1){
			bool antiAir = false;
			for (auto u : Broodwar->self()->getUnits()){
				if (u->getType().airWeapon().damageAmount() > 1){
					antiAir = true;
					break;
				}
			}
			if (antiAir){
				BuildingPlanner::enemyAir = SpecialReq::Satiated;
			}
			else if (BuildingPlanner::enemyAir == SpecialReq::NotNeeded) {//Don't undo satiated
				BuildingPlanner::enemyAir = SpecialReq::Needed;
			}
		}
	}
}

void MilitaryManager::regionUpdate(){
	MilitaryManager::allyRegions.clear();
	MilitaryManager::disputedRegions.clear();
	MilitaryManager::enemyRegions.clear();
	int i = 0;
	for (auto r : InformationManager::regions){
		switch (r.owner){
		case OwningPlayer::Self : 
			MilitaryManager::allyRegions.push_back(i);
			break;
		
		case OwningPlayer::Enemy:
			MilitaryManager::enemyRegions.push_back(i);

			break;

		case OwningPlayer::Dispute:
			MilitaryManager::disputedRegions.push_back(i);
			
			break;
		}

		i++;
	}
}

void MilitaryManager::onFrame(){
	regionUpdate();
	float ourStrength = 0;
	float theirStrength = 0;

	for (auto ou : InformationManager::costumUnits){
		if (ou->unit->getType().groundWeapon().damageAmount() < 1 && ou->unit->getType().airWeapon().damageAmount() < 1){
			continue;
		}
		if (ou->unit->getType() == BWAPI::UnitTypes::Zerg_Overlord){
			continue;
		}
		if (ou->unit->getType() == Broodwar->self()->getRace().getWorker()){
			continue;
		}

		float temp = BuildingPlanner::combatValue(ou->unit->getType())/BuildingPlanner::maxCombat*100 + BuildingPlanner::specialValue(ou->unit->getType(),true);//TODO: make custom heuristic for military
		if (temp > 0){
			ourStrength += temp;
		}
	}

	bool noBuildingsorVisible = true;
	for (auto eu : InformationManager::enemyUnits){
		if (eu.selfType.isBuilding() || eu.visible){
			noBuildingsorVisible = false;
		}
		if (eu.selfType.groundWeapon().damageAmount() < 1 && eu.selfType.airWeapon().damageAmount() < 1){
			continue;
		}
		if (eu.selfType == BWAPI::UnitTypes::Zerg_Overlord){
			continue;
		}
		if (eu.selfType == Broodwar->enemy()->getRace().getWorker()){
			continue;
		}
		float temp = BuildingPlanner::combatValue(eu.selfType)/BuildingPlanner::maxCombatEnemy*100 + BuildingPlanner::specialValue(eu.selfType,false);
		if (temp > 0){
			theirStrength += temp;
		}
	}
	std::string ourStrengthWriteout = "Our Strength: " + std::to_string(ourStrength);
	std::string theirStrengthWriteout = "Their Strength: " + std::to_string(theirStrength);

	BWAPI::Broodwar->drawTextScreen(100, 40, ourStrengthWriteout.c_str());
	BWAPI::Broodwar->drawTextScreen(100, 50, theirStrengthWriteout.c_str());

	if (noBuildingsorVisible && mainState!=MainStates::Intel){
		mainState = MainStates::Intel;
		MilitaryManager::regionCounter = 0;
		for (int i = 0; i < InformationManager::militaryUnits.size(); i++){//Deassign
			InformationManager::militaryUnits.at(i)->placement = -1;
		}
	}
	else if (!noBuildingsorVisible && mainState == MainStates::Intel){
		mainState = MainStates::Offensive;
		return;
	}


	switch (mainState){
	case MainStates::Defensive:
	{
								  if (theirStrength*1.7 < ourStrength || Broodwar->self()->supplyUsed() > 190*2){
									  mainState = MainStates::Offensive; 
									  for (int i = 0; i < InformationManager::militaryUnits.size(); i++){//Deassign
										  InformationManager::militaryUnits.at(i)->placement = -1;
									  }
									  break;
								  }
								  /*Unit proto;//Prototype
								  for (auto u : InformationManager::costumUnits){
								  if (u->unit->getType() == InformationManager::ourRace.getCenter()){
								  proto = u->unit;
								  }
								  }
								  for (auto u : InformationManager::militaryUnits){
								  u->unit->move(proto->getPosition());
								  }*/

								  //Finding offensive forces
								  std::set<BWAPI::Unit> targets;
								  for (auto ar : allyRegions){
									  for (auto u : Broodwar->getUnitsInRadius(InformationManager::regions.at(ar).self->getCenter(), 1000, BWAPI::Filter::IsEnemy)){
										  targets.insert(u);
									  }
								  }
								  for (auto dr : disputedRegions){
									  for (auto u : Broodwar->getUnitsInRadius(InformationManager::regions.at(dr).self->getCenter(), 1000, BWAPI::Filter::IsEnemy)){
										  targets.insert(u);
									  }
								  }

								  //Assigning defensive forces
								  for (int i = 0; i < InformationManager::militaryUnits.size(); i++){
									  if ((allyRegions.size() + disputedRegions.size()) == 0){
										  return;//Give up!
									  }
									  InformationManager::militaryUnits.at(i)->placement = i % (allyRegions.size()+disputedRegions.size());
								  }
								  for (auto mu : InformationManager::militaryUnits){
									  MiliHTN::defend(*mu, targets);
								  }
	}
		break;
	//END OF DEFENSIVE

	case MainStates::Offensive:
	{
								  if (theirStrength*1.1 > ourStrength && Broodwar->self()->supplyUsed() < 360){
									  mainState = MainStates::Defensive;
									  break;
								  }

								 /* EnemyUnit proto;//Prototype
								  for (auto u : InformationManager::enemyUnits){
									  if (u.visible){
										  proto = u;
										  break;
									  }
									  if (u.selfType.isBuilding()){
										  proto = u;
										  break;
									  }
								  }
								  for (auto u : InformationManager::militaryUnits){
									  u->unit->move(proto.lastSeen);
								  }*/


								  std::set<BWAPI::Unit> targets;
								  for (auto cu : InformationManager::costumUnits){
									  for (auto u : Broodwar->getUnitsInRadius(cu->unit->getPosition(), 1000, BWAPI::Filter::IsEnemy)){
										  if (u->getType() != UnitTypes::Zerg_Larva){
											  targets.insert(u);
										  }
									  }
								  }

								  for (auto cu : InformationManager::costumUnits){
									  for (auto u : Broodwar->getUnitsInRadius(cu->unit->getPosition(), 1000, BWAPI::Filter::IsAddon)){
										  if (u->getType() != UnitTypes::Zerg_Larva){
											  targets.insert(u);
										  }
									  }
								  }
								  /*if (u->getType().isAddon()){
									  Broodwar << "Addon detected!" << std::endl;
								  }*/
								  std::vector<EnemyUnit> targetBuildings;
								  for (auto eu : InformationManager::enemyUnits){
									  if (eu.selfType.isBuilding()){
										  targetBuildings.push_back(eu);
									  }
								  }
								  std::vector<MilitaryUnit*> empoweredUnits;
								  std::vector<MilitaryUnit*> weakenedUnits;
								  for (auto mu : InformationManager::militaryUnits){
									  float ourLocalStrength = 0;
									  for (auto u : Broodwar->getUnitsInRadius(mu->unit->getPosition(), 500, BWAPI::Filter::IsAlly)){
										  ourLocalStrength += BuildingPlanner::combatValue(u->getType()) / BuildingPlanner::maxCombat * 100 + BuildingPlanner::specialValue(u->getType(),false);
									  }
									  if (ourLocalStrength > ourStrength*0.7 ||ourLocalStrength > theirStrength*1.4 ||ourLocalStrength > 600){
										  empoweredUnits.push_back(mu);
									  }
									  else {
										  weakenedUnits.push_back(mu);
									  }
								  }
								  for (auto mu : empoweredUnits){
									  MiliHTN::invade(*mu, targets, targetBuildings);
								  }
								  int i = 0;
								  for (auto mu : weakenedUnits){
									  
									  i++;

									  bool combat = false;
									  for (auto u : Broodwar->getUnitsInRadius(mu->unit->getPosition(), 500, BWAPI::Filter::IsEnemy)){
										  combat = true;
										  break;
									  }
									  if (combat){
										  MiliHTN::invade(*mu, targets, targetBuildings);
										  continue;
									  }
									  i = i % weakenedUnits.size();
									  if (weakenedUnits.at(i)->unit->isFlying()){
										  i++;
										  i = i % weakenedUnits.size();
									  }
									  if (!empoweredUnits.empty()){
										  //Move closer to empowered units.
										  mu->unit->move(empoweredUnits.at(0)->unit->getPosition());
									  }
									  else if(!weakenedUnits.empty()){
										  //Move all military units closer.
										  mu->unit->move(weakenedUnits.at(i)->unit->getPosition());
									  }
								  }
	}
		break;
	//END OF OFFENSIVE

	case MainStates::Intel:
	{
							  //Assign regions
							  for (int i = 0; i < InformationManager::militaryUnits.size(); i++){//Deassign
								  if (InformationManager::militaryUnits.at(i)->placement == -1){
									  InformationManager::militaryUnits.at(i)->placement = regionCounter;
									  regionCounter++;
									  regionCounter = regionCounter%InformationManager::regions.size();
								  }

								  //Move into assigned region, then move randomly. Inspired by old code.

								  if (BWTA::getRegion(InformationManager::militaryUnits.at(i)->unit->getPosition()) ==
									  InformationManager::regions.at(InformationManager::militaryUnits.at(i)->placement).self &&
									  InformationManager::militaryUnits.at(i)->unit->isIdle()
									){
									BWTA::Polygon toScout = InformationManager::regions.at(InformationManager::militaryUnits.at(i)->placement).self->getPolygon();
									InformationManager::militaryUnits.at(i)->unit->move(toScout.at(rand() % toScout.size()));
								  }
								  else if (BWTA::getRegion(InformationManager::militaryUnits.at(i)->unit->getPosition()) !=
									  InformationManager::regions.at(InformationManager::militaryUnits.at(i)->placement).self){
									  InformationManager::militaryUnits.at(i)->unit->move(
										  InformationManager::regions.at(InformationManager::militaryUnits.at(i)->placement).self->getCenter());
								  }
							  }


	}
		break;
	//END OF INTEL
	}

}

