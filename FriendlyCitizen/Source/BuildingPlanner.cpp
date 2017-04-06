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
		for (auto ot : InformationManager::ourTech){
			if (ot.selfType == ut){
				currentNodes.push_back(ot);
			}
		}
	}//Make our initial nodes that cannot be planned, as they're already fulfilled.

	std::vector<TechNode> possibleNodes = currentNodes;
	for (int i = 0; i < currentNodes.size(); i++){
		for (int i2 = 0; i2 < currentNodes.at(i).effect.size(); i2++){
			possibleNodes.push_back(currentNodes.at(i).effect.at(i2));
		}
	}//PossibleNodes contain all possible things to be constructed.

	std::vector<int> possibleNodesHeuristics;

	for (int i = 0; i < possibleNodes.size(); i++){
		possibleNodesHeuristics.push_back(heuristic(possibleNodes.at(i)));
	}

	int chosenID = 0;
	int chosenHeuristicsValue;
	for (int i = 0; i < possibleNodesHeuristics.size(); i++){
		if (chosenHeuristicsValue > possibleNodesHeuristics.at(i)){
			chosenHeuristicsValue = possibleNodesHeuristics.at(i);
			chosenID = i;
		}
	}

	//TODO: Handle unresearched technologies.
	//TODO: Order chosenID's type to be constructed.
}

int BuildingPlanner::heuristic(TechNode node){//Temporary function to calculate simple heuristical values
	//TODO: Handle unresearched technologies.
	int value = 0;
	//Plan:
	//Saturation heuristics: Too many of the same type of unit is bad.

	//Balance heuristics: Too many units of one of the three catagories is probably bad.
	//Price heuristics: Relative expensive things is bad. Relative expensiveness relies on income.
	//Tech satiation: Tech buildings that already exists are pointless.
	//
	//Enemy analysis: A whole range of heuristics not to be made as of yet about enemy analysis heuristical results.
	//State heuristics: At some point in time, a state might ask for a certain unit. Maybe.
	//Infrastructure demand: Similar to the above, pylons and creep producers might be necessary for future construction.
	return(node.selfType.gasPrice() + node.selfType.mineralPrice());
}