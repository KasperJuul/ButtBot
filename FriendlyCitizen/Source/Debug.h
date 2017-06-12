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
	static void writeLog(std::vector<std::string> message, std::string fileName, std::string folderName);
	static void writeLog(std::set<std::string> message, std::string fileName, std::string folderName);
	static void writeTimedLog(std::string message, std::string fileName, std::string folderName);
	static void screenInfo();
	static std::vector<std::string> errorLog;
	static void errorLogMessages(std::string);
	static void endWriteLog();
};

