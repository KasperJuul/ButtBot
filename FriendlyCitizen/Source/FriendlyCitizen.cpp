#include "FriendlyCitizen.h"
#include <BWTA.h>
#include <iostream>
#include <cppunit/TestCase.h>
#include "InformationManager.h"
#include "ResourceManager.h"
#include "IntelManager.h"

using namespace BWAPI;
using namespace Filter;

//Debug settings
bool debug = true;
bool defog = false;
int optimisation = 2; //Using built-in bwapi optimisation

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
	}
	Broodwar->setLocalSpeed(56);

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
	//Graphical functions might have to go before latency frames in order to avoid stuttering.
	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;
	
	//Onframe functionality.
	IntelManager::ScoutOnFrame();
	ResourceManager::onFrame();
}

void FriendlyCitizen::onSendText(std::string text)
{

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
}
