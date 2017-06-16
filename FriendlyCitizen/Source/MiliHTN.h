#pragma once
#include <BWAPI.h>
#include "MilitaryManager.h"
#include "CostumUnit.h"
#include "InformationManager.h"
#include "MilitaryManager.h"
//#include <BWTA.h>
enum ActionDec{Move,Attack,Patrol,Hold,None};

struct PositionOrUnit{
	BWAPI::Position position;
	BWAPI::Unit unit;
	bool isUnit = false;
	bool isPosition = false;
};

class MiliHTN
{
public:
	static BWAPI::Unit chooseTarget(BWAPI::Unit attacker, std::set<BWAPI::Unit> targets);
	static void kiteAttack(BWAPI::Unit kiter, BWAPI::Unit target);
	static void defend(MilitaryUnit defender, std::set<BWAPI::Unit> targets);
	static void invade(MilitaryUnit invader, std::set<BWAPI::Unit> targets, std::vector<EnemyUnit> targetBuildings);
	static bool attack(BWAPI::Unit attacker, BWAPI::Unit target);
};

