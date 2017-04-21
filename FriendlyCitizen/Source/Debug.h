#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

class Debug
{
public:
	static void writeLog(std::string message, std::string fileName, std::string folderName);
	static void writeTimedLog(std::string message, std::string fileName, std::string folderName);
	static void screenInfo();
};

