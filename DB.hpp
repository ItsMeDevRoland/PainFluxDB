//
//
// Shares the DB.cpp's functions and etc to other cpp files!
// THIS DOES NOT HAVE ANYTHING TO MODIFY, DO NOT TOUCH UNLESS YOU KNOW WHAT YOU'RE DOING
//
//

#pragma once
#include <string>
#include "struct.hpp"
using namespace std;

int getDBSize();
bool CreateNewDatabase(string MessageOverride = "");
Database returnQueryADB(string Name = "");
string returnNameQueryADB(string Name = "");
Containers returnAllDB(string Name = "");
