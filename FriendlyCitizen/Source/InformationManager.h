#pragma once
#include <BWAPI.h>
#include <vector>

using namespace BWAPI;

static class InformationManager
{
public:
	//Analytial functions
	static void StartAnalysis();

	//Information
	static Race ourRace;
	static Unit firstNexus; //Swap out with better, generalized functionality later
	static std::vector<Unit> firstWorkers; //Swap out with better, generalized functionality later
};

