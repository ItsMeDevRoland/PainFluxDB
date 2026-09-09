// Welcome to PainFluxDB, the embeddable databaseengine led by
// Roland Sibal, {INSERT NAME FOR CONTRIBUTORS}
// Don't ask all the decision made by us when we made the engine
/*
         ################
        #####################
        ###---------------####
       *###--------------######
       ###------      #########
       ###------      #########
      ###*-------------########
      ###--------------########                 PainFluxDB - Ver. 0.0.1
      ###-------------#######                 LICENSED UNDER APACHE 2.0
     ###=-----+#############              Made for sir Exekiel, Data Structure
     ###------##########
     ###------
    #########
    #########
   #########
*/



// IMPORTS
#include "main.hpp"
#include "config.hpp"
#include "struct.hpp"
#include "utility.hpp"
#include "DB.hpp"

#include <iostream>
#include <string>
using namespace std;

// DO NOT MOVE DOWN UNLESS YOU KNOW WHAT YOU'RE DOING!
// If you're invoking a function, use the available and adviced function above!
// WARNING: ALL TABLES AND NOTES HAVE A 100 LIMIT... MODIFY THE LIMITS AT THE
// CONFIG.HPP!



extern Database databaseContainer[DatabaseLimit];

extern Containers ArrayContainer;

extern Database* DatabaseSelected;




void showAllDB() {
  cout << " || ALL DATABASE STORED ||" << endl;

  Containers DatabaseList = returnAllDB();
  for (int i = 0; i < getDBSize(); i++) {
    cout << " || " << DatabaseList.DatabaseArray[i]->DatabaseName << " ||"
         << endl;
  }
}

string showHelp() {
  if (EnableDevTool == false) {
    string Help = R"(
        Welcome To PainFluxDB, This is a Help Command. Its Job is to ensure and give you knowledge!

        HOW TO START?
        -- using showHelp() shows this Menu, and show the help to the user, Enabling the Devtool give the help of the Cli

    )";
    return Help;

  } else {
    string Help = R"(
        Welcome To PainFluxDB CLI, This is the DevTool CLI help, below are the Calls you can do within the CLI provided:

        CREATEDB - creates a new database,
        EXIT - exit off the CLI
        HELP - opens this menu
        SHOWALLDB - show all database stored within the system
        USE - use a database, REQUIRED TO MODIFY OR CREATE A NEW TABLE(CREATETB)

    )";
    return Help;
  }
}

void HandleParser(string Input) {

  if (Input == "CREATEDB") {
    CreateNewDatabase();
  } else if (Input == "SHOWALLDB") {
      showAllDB();
  } else if (Input == "HELP"){
      string Help = showHelp();
      cout << Help << endl;
  } else if (Input == "USE"){
      UseDB();
  };
}

bool DevTool() {
  string Selection;
  string StandardizedSelection;
  int KillDevTool = 0;

  while (KillDevTool != 1) {
    cout << "Welcome to CLI of PainFlux" << endl;
    cout << "Stuck? Type 'HELP' and get help!" << endl;
    cin >> Selection;

    StandardizedSelection = standardizedString(Selection);
    if (StandardizedSelection != "EXIT") {
      HandleParser(StandardizedSelection);
    } else {
      cout << "Bye bye!";
      KillDevTool = 1;
      continue;
    }
  }

  return true;
};

// Enable if you want to use the Developer CLI
int main() {
  // Check if DevTool is true, if true enable the debugging layer
  if (EnableDevTool == true) {
    DevTool();
  };

  return 0;
}
