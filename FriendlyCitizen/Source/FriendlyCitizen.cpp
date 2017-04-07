#include "FriendlyCitizen.h"
#include <BWTA.h>
#include <iostream>
#include <cppunit/TestCase.h>
#include "InformationManager.h"
#include "ResourceManager.h"
#include "IntelManager.h"
#include "BuildingPlacer.h"
#include "BuildingPlanner.h"

using namespace BWAPI;
using namespace Filter;

//Debug settings
bool dbg_mode = true;
bool debug = true;
bool defog = false;
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
		//IntelManager::StartScouting();

		analyzed = false;
		analysis_just_finished = false;
	}
	Broodwar->setLocalSpeed(41);
}

void FriendlyCitizen::onEnd(bool isWinner)
{
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

	if (Broodwar->getFrameCount() % 40 == 0){
		minelog += std::to_string(Broodwar->getFrameCount()) +
		", " + std::to_string(Broodwar->self()->gatheredMinerals()) +
		", " + std::to_string(InformationManager::wrkUnits.size()) + "\n";
	}

	/*for (unsigned int i = 0; i < ResourceManager::minPatches.size(); i++){
		std::string s = "M" + std::to_string(i) + ": ";
		for (unsigned int j = 0; j < ResourceManager::minPatches.at(i).workers.size(); j++){
			s += "[" + std::to_string(ResourceManager::minPatches.at(i).workers.at(j)->getID()) + "] ";
		}
		Broodwar->drawTextScreen(20, 40 + (i*10), s.c_str());
		Broodwar->drawTextMap(ResourceManager::minPatches.at(i).unit->getPosition(), "M%d", i);
	}*/
	
	for (auto &u : Broodwar->self()->getUnits()){
		if (u->getType().isWorker()){
			Broodwar->drawTextMap(u->getPosition(),"%d", u->getID());
		}
		if (u->getType().isResourceDepot()){
			Broodwar->drawTextMap(u->getPosition(), "%d", u->getID());
		}
	}

	//std::string w = "Workers:";
	std::string derp = "Base 1:  " + std::to_string(InformationManager::centers.at(0).unit->getID());
	std::string w = "Workers: " + std::to_string(InformationManager::centers.at(0).wrkUnits.size());
	for (unsigned int i = 0; i < InformationManager::wrkUnits.size(); i++){
		//w += " [" + std::to_string(ResourceManager::wrkUnits.at(i).unit->getID()) + "]";
		Position po = Position(InformationManager::wrkUnits.at(i).unit->getPosition().x - 10, InformationManager::wrkUnits.at(i).unit->getPosition().y - 10);
		Broodwar->drawTextMap(po, InformationManager::wrkUnits.at(i).status.c_str());
	}
	Broodwar->drawTextScreen(20, 30, derp.c_str());
	Broodwar->drawTextScreen(20, 40, w.c_str());

	//Broodwar->drawTextScreen(20, 40, "M1: %d %d", ResourceManager::minPatches.at(0).workers[0]->getID(), ResourceManager::minPatches.at(0).workers[1]->getID());
	//Broodwar->drawTextScreen(20, 50, "M2: %d", ResourceManager::minPatches.at(1).workers[0]->getID());
	if (dbg_mode){
		Broodwar->drawTextScreen(20, 0, "Supply used: %d", Broodwar->self()->supplyUsed() / 2);
		Broodwar->drawTextScreen(20, 20, "Supply total: %d", BWAPI::Broodwar->self()->supplyTotal() / 2);
		//ResourceManager::drawMinCircles();
	}
	
	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	//BWTA draw
	//if (analyzed)
		//drawTerrainData();

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
	//IntelManager::ScoutOnFrame();
	BuildingPlanner::plannerOnFrame();
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
	InformationManager::OnNewUnit(unit);
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
	if (unit->getType().isResourceDepot()){
		Center temp;
		temp.unit = unit;
		temp.wrkUnits.clear();
		BuildingPlacer::xpandIsBeingBuild = false;
	}
	if (unit->getType().isWorker()){
		workerUnit temp;
		temp.unit = unit;
		temp.status = "Idle";
		Unit center = unit->getClosestUnit(IsResourceDepot);
		Broodwar << "center = " << std::to_string(center->getID()) << std::endl;
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