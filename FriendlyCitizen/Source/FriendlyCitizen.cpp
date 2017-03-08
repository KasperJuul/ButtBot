#include "FriendlyCitizen.h"
#include <BWTA.h>
#include <iostream>
#include <cppunit/TestCase.h>
#include "InformationManager.h"
#include "ResourceManager.h"

using namespace BWAPI;
using namespace Filter;

//Debug settings
bool dbg_mode = true;
bool debug = true;
bool defog = false;
int optimisation = 2; //Using built-in bwapi optimisation

bool analyzed;
bool analysis_just_finished;

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
		InformationManager::StartAnalysis();
		ResourceManager::onStart();
	}
	
	analyzed = false;
	analysis_just_finished = false;
}

void FriendlyCitizen::onEnd(bool isWinner)
{
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

	Broodwar->drawTextScreen(20, 0, "Supply used: %d", Broodwar->self()->supplyUsed() / 2 );
	Broodwar->drawTextScreen(20, 20, "Supply total: %d", BWAPI::Broodwar->self()->supplyTotal() / 2 );
	
	ResourceManager::drawMinCircles();

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	//BWTA draw
	if (analyzed)
		drawTerrainData();

	if (analysis_just_finished)
	{
		Broodwar << "Finished analyzing map." << std::endl;;
		analysis_just_finished = false;
	}

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	

	ResourceManager::onFrame();
	//// Iterate through all the units that we own
	//for (auto &u : Broodwar->self()->getUnits())
	//{
	//	// Ignore the unit if it no longer exists
	//	// Make sure to include this block when handling any Unit pointer!
	//	if (!u->exists())
	//		continue;

	//	// Ignore the unit if it has one of the following status ailments
	//	if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
	//		continue;

	//	// Ignore the unit if it is in one of the following states
	//	if (u->isLoaded() || !u->isPowered() || u->isStuck())
	//		continue;

	//	// Ignore the unit if it is incomplete or busy constructing
	//	if (!u->isCompleted() || u->isConstructing())
	//		continue;


	//	// Finally make the unit do some stuff!


	//	// If the unit is a worker unit
	//	if (u->getType().isWorker())
	//	{
	//		// if our worker is idle
	//		if (u->isIdle())
	//		{
	//			// Order workers carrying a resource to return them to the center,
	//			// otherwise find a mineral patch to harvest.
	//			if (u->isCarryingGas() || u->isCarryingMinerals())
	//			{
	//				u->returnCargo();
	//			}
	//			else if (!u->getPowerUp())  // The worker cannot harvest anything if it
	//			{                             // is carrying a powerup such as a flag
	//				// Harvest from the nearest mineral patch or gas refinery
	//				if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery)))
	//				{
	//					// If the call fails, then print the last error message
	//					Broodwar << Broodwar->getLastError() << std::endl;
	//				}

	//			} // closure: has no powerup
	//		} // closure: if idle

	//	}
	//	else if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
	//	{

	//		// Order the depot to construct more workers! But only when it is idle.
	//		if (u->isIdle() && !u->train(u->getType().getRace().getWorker()))
	//		{
	//			// If that fails, draw the error at the location so that you can visibly see what went wrong!
	//			// However, drawing the error once will only appear for a single frame
	//			// so create an event that keeps it on the screen for some frames
	//			Position pos = u->getPosition();
	//			Error lastErr = Broodwar->getLastError();
	//			Broodwar->registerEvent([pos, lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
	//				nullptr,    // condition
	//				Broodwar->getLatencyFrames());  // frames to run

	//			// Retrieve the supply provider type in the case that we have run out of supplies
	//			UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
	//			static int lastChecked = 0;

	//			// If we are supply blocked and haven't tried constructing more recently
	//			if (lastErr == Errors::Insufficient_Supply &&
	//				lastChecked + 400 < Broodwar->getFrameCount() &&
	//				Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
	//			{
	//				lastChecked = Broodwar->getFrameCount();

	//				// Retrieve a unit that is capable of constructing the supply needed
	//				Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
	//					(IsIdle || IsGatheringMinerals) &&
	//					IsOwned);
	//				// If a unit was found
	//				if (supplyBuilder)
	//				{
	//					if (supplyProviderType.isBuilding())
	//					{
	//						TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
	//						if (targetBuildLocation)
	//						{
	//							// Register an event that draws the target build location
	//							Broodwar->registerEvent([targetBuildLocation, supplyProviderType](Game*)
	//							{
	//								Broodwar->drawBoxMap(Position(targetBuildLocation),
	//									Position(targetBuildLocation + supplyProviderType.tileSize()),
	//									Colors::Blue);
	//							},
	//								nullptr,  // condition
	//								supplyProviderType.buildTime() + 100);  // frames to run

	//							// Order the builder to construct the supply structure
	//							supplyBuilder->build(supplyProviderType, targetBuildLocation);
	//						}
	//					}
	//					else
	//					{
	//						// Train the supply provider (Overlord) if the provider is not a structure
	//						supplyBuilder->train(supplyProviderType);
	//					}
	//				} // closure: supplyBuilder is valid
	//			} // closure: insufficient supply
	//		} // closure: failed to train idle unit

	//	}

	//} // closure: unit iterator
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
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());


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
	if (dbg_mode){
		Broodwar <<  unit->getType().getName() << " completed and ready!" << std::endl;
	}
	if (BWAPI::Broodwar->self()->supplyUsed() + 4 >= BWAPI::Broodwar->self()->supplyTotal())
	{
		UnitType supplyProviderType = unit->getType().getRace().getSupplyProvider();
		TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, unit->getTilePosition());
		unit->build(supplyProviderType, targetBuildLocation);
		Broodwar << "Worker is building supply unit" << std::endl;
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