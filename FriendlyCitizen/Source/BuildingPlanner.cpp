#include "BuildingPlanner.h"
#include "InformationManager.h"


void BuildingPlanner::plannerOnFrame(){

	makePlan();//TEST, REMOVE LATER
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
				bool valid = true;
				for (auto precon : ot.precondition){
					if (!precon.exists) valid = false;
				}
				if(valid) currentNodes.push_back(ot);
			}
			//Broodwar << ut.getName() << " : " << ot.selfType.getName() << std::endl;
		}
		//Broodwar << ut.getName() << std::endl;
	}//Make our initial nodes that cannot be planned, as they're already fulfilled.

	//Broodwar << std::to_string(currentNodes.size()).c_str() << std::endl;
	if (currentNodes.size() == 0){
		return;
	}

	std::vector<TechNode> possibleNodes = currentNodes;
	for (int i = 0; i < currentNodes.size(); i++){
		for (int i2 = 0; i2 < currentNodes.at(i).effect.size(); i2++){
			bool valid = true;
			for (auto precon : currentNodes.at(i).effect.at(i2).precondition){
				if (!precon.exists) valid = false;
			}
			if (valid) possibleNodes.push_back(currentNodes.at(i).effect.at(i2));
		}
	}//PossibleNodes contain all possible things to be constructed.

	std::vector<TechNode> newPossibleNodes;
	for (int i = 0; i < possibleNodes.size(); i++){//Temporary solution for derp comsat
		bool valid = true;
		for (auto precon : possibleNodes.at(i).precondition){
			if (!precon.exists) valid = false;//HOW DOESN'T THIS WORK?!
		}
		if (valid) newPossibleNodes.push_back(possibleNodes.at(i));
	}
	std::string derp;
	for (int i = 0; i < newPossibleNodes.size(); i++){
		for (auto FUCKTHISSHIT : newPossibleNodes.at(i).precondition){
			derp += FUCKTHISSHIT.selfType.getName() + "\n";
		}
	}
	Debug::writeLog(derp,"FAAAAAK","FAAAAK");
	
	possibleNodes = newPossibleNodes;

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
	Broodwar << possibleNodes.at(chosenID).selfType.getName() << " : " << std::to_string(chosenHeuristicsValue).c_str() << std::endl;
}

int BuildingPlanner::heuristic(TechNode node){//Temporary function to calculate simple heuristical values.
	//TODO: Handle unresearched technologies.
	//TODO: Refactor/clean up code.
	int value = 0;
	//Plan:
	//Saturation heuristics: Too many of the same type of unit is bad.
	int saturation = 0;
	for (auto us : InformationManager::ourUnits){
		if (us.self->getType() == node.selfType) saturation += 10;
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
	}
	else {//Tech
		balanceValue += (int)(0 - balanceEconomy * 0.3 - balanceMilitary * 0.2 + balanceTech * 0.5);
		//Tech satiation: Tech buildings that already exists are pointless.
		for (auto ut : InformationManager::ourUnitTypes){
			if (ut == node.selfType){
				value += 1000;
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
	return(value);
}