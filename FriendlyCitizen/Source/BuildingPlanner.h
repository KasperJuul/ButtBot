#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "InformationManager.h"

class BuildingPlanner{
public:
	static BWAPI::UnitType makePlan();
	static void plannerOnFrame();
	static int heuristic(TechNode node);
	static std::string plan;
};