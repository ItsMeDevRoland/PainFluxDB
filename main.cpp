// Welcome to PainFluxDB, the embeddable database engine led by
// Roland Sibal, {INSERT NAME FOR CONTRIBUTORS}
// Don't ask about the decisions made by us when we made the engine
/*
         ################
        #####################
        ###---------------####
       *###--------------######
       ###------      #########
       ###------      #########
      ###*-------------########
      ###--------------########                 PainFluxDB - Ver. 0.1.0
      ###-------------#######                 LICENSED UNDER APACHE 2.0
     ###=-----+#############              Made for sir Exekiel, Data Structure
     ###------##########
     ###------
    #########
    #########
   #########
*/

// This file is a DEVELOPMENT HARNESS. The real program is built by the
// application team on top of the engine headers. Keep it small and disposable.

#include "DB.hpp"
#include "Row.hpp"
#include "config.hpp"
#include "main.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "stats.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"

#include <iostream>
#include <string>
using namespace std;

extern Database databaseContainer[DatabaseLimit];
extern Containers ArrayContainer;
extern Database *DatabaseSelected;

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

        CREATEDB - creates a new database
        EXIT - exit off the CLI
        HELP - opens this menu
        SHOWALLDB - show all database stored within the system
        USE - use a database, REQUIRED TO MODIFY OR CREATE A NEW TABLE(CREATETB)
        DELETEDB - delete a database you named

    )";
    return Help;
  }
}

void HandleParser(string Input) {

  if (Input == "CREATEDB") {
    CreateNewDatabase();
  } else if (Input == "SHOWALLDB") {
    showAllDB();
  } else if (Input == "HELP") {
    string Help = showHelp();
    cout << Help << endl;
  } else if (Input == "USE") {
    UseDB();
  } else if (Input == "DELETEDB") {
    DeleteDatabase();
  };
}

bool DevTool() {
  string Selection;
  string StandardizedSelection;
  int KillDevTool = 0;

  while (KillDevTool != 1) {
    cout << "Welcome to CLI of PainFlux" << endl;
    cout << "Stuck? Type 'HELP' and get help!" << endl;

    // Guard against EOF/failed input. Without this, "cin >> Selection"
    // fails, Selection stays empty, and the loop spins forever printing
    // the banner millions of times.
    if (!(cin >> Selection)) {
      cout << endl << "[ERROR] No more input, closing the CLI." << endl;
      break;
    }

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
