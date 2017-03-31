#include "BuildingPlanner.h"
#include "InformationManager.h"


void BuildingPlanner::plannerOnFrame(){
	if (false){ //If there is a plan...
		//Execute plan
	}
	else {//... Else, make a new plan
		BuildingPlanner::makePlan();
	}
}

void BuildingPlanner::makePlan(){
	std::vector<TechNode> currentNodes;
	for (auto ut : InformationManager::ourUnitTypes){
		for (auto t : InformationManager::ourTech){
			if (t.selfType == ut){
				currentNodes.push_back(ut);
			}
		}
	}
}