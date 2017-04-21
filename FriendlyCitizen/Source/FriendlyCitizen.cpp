#include "FriendlyCitizen.h"
#include <BWTA.h>
#include <iostream>
#include <cppunit/TestCase.h>
#include "InformationManager.h"
#include "ResourceManager.h"
#include "IntelManager.h"
#include "BuildingPlacer.h"
#include "BuildingPlanner.h"
#include "MilitaryManager.h"

using namespace BWAPI;
using namespace Filter;

//Debug settings
bool dbg_mode = true;
bool debug = true;
bool defog = true;
bool analyzed = false;
bool analysis_just_finished;
int optimisation = 2; //Using built-in bwapi optimisation
std::string FriendlyCitizen::minelog = "";

void FriendlyCitizen::onStart()
{
	BWTA::analyze();
	BWTA::readMap();
	//Settings
	if (debug){//Allows us to write commands
		Broodwar->enableFlag(Flag::UserInput);
	}
	if (defog){//Allows our bot to attain an unfair information advantage
		Broodwar->enableFlag(Flag::CompleteMapInformation);
	}
	Broodwar->setCommandOptimizationLevel(optimisation);//I've no idea

	if (Broodwar->isReplay())// Check if this is a replay
	{
		Broodwar << "FriendlyCitizen is spectating the game." << std::endl;
	}
	else // if this is not a replay
	{
		Broodwar << "We are playing as" << Broodwar->self()->getRace() << std::endl;
		//Setup functions
		InformationManager::StartAnalysis();//MUST be first!
		ResourceManager::onStart();
		IntelManager::StartScouting();

		analyzed = false;
		analysis_just_finished = false;
	}
	Broodwar->setLocalSpeed(41);

	//Broodwar->sendText("black sheep wall");
	//Broodwar->sendText("operation cwal");
	//Broodwar->sendText("black sheep wall");
	

}

void FriendlyCitizen::onEnd(bool isWinner)
{
	for (unsigned int i = 0; i < InformationManager::ourTech.size(); i++){
		std::string temp = "This unit builds:\n";
		for (unsigned int i2 = 0; i2 < InformationManager::ourTech.at(i).effect.size(); i2++){
			temp += InformationManager::ourTech.at(i).effect.at(i2)->selfType.c_str();
			temp += "\n";
		}
		temp += "\nThis unit requires:\n";
		for (unsigned int i2 = 0; i2 < InformationManager::ourTech.at(i).precondition.size(); i2++){
			temp += InformationManager::ourTech.at(i).precondition.at(i2)->selfType.c_str();
			temp += "\n";
		}
		temp += "\nThis unit exists: " + std::to_string(InformationManager::ourTech.at(i).exists);
		Debug::writeLog(temp.c_str(), InformationManager::ourTech.at(i).selfType.getName().c_str(), InformationManager::ourRace.getName().c_str());
	}
	Debug::writeLog(ResourceManager::log, "QueueLog", "Logs");
	Debug::writeLog(FriendlyCitizen::minelog, "minLog", "Logs");
	// Called when the game ends
	if (isWinner)
	{
		// Log your win here!
	}
}

void FriendlyCitizen::onFrame()
{
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

	if (dbg_mode){
		Debug::screenInfo();
	}

	if (Broodwar->getFrameCount() % 40 == 0){
		minelog += std::to_string(Broodwar->getFrameCount()) +
			", " + std::to_string(Broodwar->self()->gatheredMinerals()) +
			", " + std::to_string(InformationManager::wrkUnits.size()) + "\n";
	}

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;
	drawTerrainData();
	//BWTA draw
	if (analyzed)
		drawTerrainData();

	if (analysis_just_finished)
	{
		Broodwar << "Finished analyzing map." << std::endl;;
		analysis_just_finished = false;
	}
	//Graphical functions might have to go before latency frames in order to avoid stuttering.
	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;
	
	//Onframe functionality.
	BuildingPlacer::onFrame();
	ResourceManager::onFrame();
	IntelManager::ScoutOnFrame();
	MilitaryManager::onFrame();
	
}

void FriendlyCitizen::onSendText(std::string text)
{
	if (text == "/analyze") {
		if (analyzed == false) {
			Broodwar << "Analyzing map... this may take a minute" << std::endl;;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
		}
	}
	else if (text == "/dbgmode"){
		dbg_mode = !dbg_mode;
		if (dbg_mode){
			Broodwar << "Debug mode ON" << std::endl;;
		}
		else{
			Broodwar << "Debug mode OFF" << std::endl;;
		}
	}
	else {

		// Send the text to the game if it is not being processed.
		Broodwar->sendText("%s", text.c_str());
	}


	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!

}

void FriendlyCitizen::onReceiveText(BWAPI::Player player, std::string text)
{
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FriendlyCitizen::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void FriendlyCitizen::onNukeDetect(BWAPI::Position target)//Infomanager low priority
{

	// Check if the target is a valid position
	if (target)
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void FriendlyCitizen::onUnitDiscover(BWAPI::Unit unit)
{
	InformationManager::OnNewUnit(unit);
}

void FriendlyCitizen::onUnitEvade(BWAPI::Unit unit)
{
}

void FriendlyCitizen::onUnitShow(BWAPI::Unit unit)
{
}

void FriendlyCitizen::onUnitHide(BWAPI::Unit unit)
{
}

void FriendlyCitizen::onUnitCreate(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self()){
		if (unit->getType().isBuilding()){
			int itr = 0;
			for (auto &b : InformationManager::orderedBuildings){
				if (b == unit->getType()){
					InformationManager::orderedBuildings.erase(InformationManager::orderedBuildings.begin() + itr);
					break;
				}
				itr++;
			}
			InformationManager::reservedMinerals -= unit->getType().mineralPrice();
			InformationManager::reservedGas -= unit->getType().gasPrice();
		}
	}
	//InformationManager::OnNewUnit(unit);
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void FriendlyCitizen::onUnitDestroy(BWAPI::Unit unit)
{
	InformationManager::OnUnitDestroy(unit);
}

void FriendlyCitizen::onUnitMorph(BWAPI::Unit unit)
{
	InformationManager::OnUnitDestroy(unit);
	InformationManager::OnNewUnit(unit);
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
	if (unit->getPlayer() == Broodwar->self()){
		if (unit->getType().isBuilding()){
			bool found = false;
			for (auto &c : InformationManager::centers){
				int itr = 0;
				for (auto &w : c.wrkUnits){
					if (unit == w.unit){
						c.wrkUnits.erase(c.wrkUnits.begin() + itr);
						found = true;
						break;
					}
					itr++;
				}
				if (found){
					break;
				}
			}
		}
		Broodwar << "unit " << std::to_string(unit->getID()) << " has morphed into " << unit->getType().toString() << std::endl;
	}
}

void FriendlyCitizen::onUnitRenegade(BWAPI::Unit unit)
{
}

void FriendlyCitizen::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void FriendlyCitizen::onUnitComplete(BWAPI::Unit unit)
{

	InformationManager::OnNewUnit(unit);
	if (unit->getPlayer() == Broodwar->self()){
		if (unit->getType().isResourceDepot()){
			Center temp;
			temp.unit = unit;
			temp.wrkUnits.clear();
			bool already_exists = false;
			for (auto &c : InformationManager::centers){
				if (c.unit == unit)
					already_exists = true;
			}
			if (!already_exists){
				InformationManager::centers.push_back(temp);
			}
			BuildingPlacer::xpandIsBeingBuild = false;
		}
		if (unit->getType().isWorker()){
			workerUnit temp;
			temp.unit = unit;
			temp.status = "Idle";
			Unit center = unit->getClosestUnit(IsResourceDepot);
			for (auto &c : InformationManager::centers){
				if (c.unit == center){
					c.wrkUnits.push_back(temp);
				}
			}
			//InformationManager::wrkUnits.push_back(temp);
		}

		if (unit->getType() == Broodwar->self()->getRace().getSupplyProvider()){
			BuildingPlacer::supplyProviderIsBeingBuild = false;

		}
	}
}

DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

void FriendlyCitizen::drawTerrainData()
{
	//we will iterate through all the base locations, and draw their outlines.
	for (const auto& baseLocation : BWTA::getBaseLocations()) {
		TilePosition p = baseLocation->getTilePosition();

		//draw outline of center location
		Position leftTop(p.x * TILE_SIZE, p.y * TILE_SIZE);
		Position rightBottom(leftTop.x + 4 * TILE_SIZE, leftTop.y + 3 * TILE_SIZE);
		Broodwar->drawBoxMap(leftTop, rightBottom, Colors::Blue);

		//draw a circle at each mineral patch
		for (const auto& mineral : baseLocation->getStaticMinerals()) {
			Broodwar->drawCircleMap(mineral->getInitialPosition(), 30, Colors::Cyan);
		}

		//draw the outlines of Vespene geysers
		for (const auto& geyser : baseLocation->getGeysers()) {
			TilePosition p1 = geyser->getInitialTilePosition();
			Position leftTop1(p1.x * TILE_SIZE, p1.y * TILE_SIZE);
			Position rightBottom1(leftTop1.x + 4 * TILE_SIZE, leftTop1.y + 2 * TILE_SIZE);
			Broodwar->drawBoxMap(leftTop1, rightBottom1, Colors::Orange);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if (baseLocation->isIsland()) {
			Broodwar->drawCircleMap(baseLocation->getPosition(), 80, Colors::Yellow);
		}
	}

	//we will iterate through all the regions and ...
	for (const auto& region : BWTA::getRegions()) {
		// draw the polygon outline of it in green
		BWTA::Polygon p = region->getPolygon();
		for (size_t j = 0; j < p.size(); ++j) {
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLineMap(point1, point2, Colors::Green);
		}
		// visualize the chokepoints with red lines
		for (auto const& chokepoint : region->getChokepoints()) {
			Position point1 = chokepoint->getSides().first;
			Position point2 = chokepoint->getSides().second;
			Broodwar->drawLineMap(point1, point2, Colors::Red);
		}
	}
}