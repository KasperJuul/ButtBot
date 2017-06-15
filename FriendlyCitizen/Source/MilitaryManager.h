#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <vector>
#include <deque>
#include <algorithm>
#include "MiliHTN.h"
enum MainStates{ Defensive,Offensive,Intel };

class MilitaryManager
{
public:
	static std::vector<int> enemyRegions;
	static std::vector<int> disputedRegions;
	static std::vector<int> allyRegions;
	static int regionCounter;

	static void regionUpdate();
	static MainStates mainState;
	static void onFrame();
};

