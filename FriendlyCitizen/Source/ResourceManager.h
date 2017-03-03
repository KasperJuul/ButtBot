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
			std::deque<BWAPI::Unit> workers;
		};	
	static void onStart();
	static void onFrame();
	static void drawMinCircles();
	static void findMinPatches();


	static int workTime(mineralPatch m);
	static int workTime(BWAPI::Unit unit, mineralPatch m, int n);
	static int workTime(std::deque<BWAPI::Unit> q);

	

	
	static BWTA::BaseLocation* mainBase;
	static BWAPI::Unitset mins;
};

