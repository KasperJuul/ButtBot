#pragma once
#include "Debug.h"
#include <fstream>
#include <direct.h>
#include <time.h>

void Debug::writeLog(std::string message, std::string fileName, std::string folderName){
	_mkdir(folderName.c_str());
	std::ofstream writeTo;
	writeTo.open(folderName + "\\" + fileName + ".txt");
	writeTo << message+"\n";
	writeTo.close();
}

void Debug::writeTimedLog(std::string message, std::string fileName, std::string folderName){//UNTESTED
	_mkdir(folderName.c_str());
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	std::ofstream writeTo;
	writeTo.open(folderName + "\\" + asctime(timeinfo) + "; " + fileName + ".txt");
	writeTo << message + "\n";
	writeTo.close();
}