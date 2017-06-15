#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <vector>
#include <deque>
#include <algorithm>


// Remember not to use "Broodwar" in any global class constructor!

class ResourceManager
{
public:
	struct mineralPatch{
		BWAPI::Unit unit;
		std::string name;
		std::deque<BWAPI::Unit> workers;
		int center;
	};	
	

	static void onStart();
	static void onFrame();
	static void buildPylonsNProbes();
	static void buildPylonsNProbes2();
	static void drawMinCircles();
	static void findMinPatches();

	static void stdGather();
	static void qGather();
	static void gasGather();

	static int workTime(mineralPatch m);
	static int workTime(BWAPI::Unit unit, mineralPatch m, int n);
	static int workTime2(BWAPI::Unit unit, mineralPatch m, int n);
	static int roundTrip(BWAPI::Unit u , mineralPatch m);
	static mineralPatch* roundTrip_min(BWAPI::Unit u, std::vector<ResourceManager::mineralPatch>* patches);

	

	static std::vector<ResourceManager::mineralPatch> minPatches;
	static std::string log;
};

