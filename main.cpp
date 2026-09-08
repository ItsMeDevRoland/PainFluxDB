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
#include <iostream>
#include <string>
using namespace std;

const int TableColLimit =
    100; // Since we're Using Array instead of Vector, Limit is placed here,
         // modify depending on your USE CASE
const int TableRowLimit = TableColLimit; // Row limit per column, DO NOT MODIFY!
const int DatabaseLimit = 100; // Database Amount PainFluxDB can accomodate
const int TableLimit = 100;    // Table per database

const bool EnableDevTool = true; // enable the CLI built in within PainFluxDB

// DO NOT MOVE DOWN UNLESS YOU KNOW WHAT YOU'RE DOING!
// If you're invoking a function, use the available and adviced function above!
// WARNING: ALL TABLES AND NOTES HAVE A 100 LIMIT... MODIFY THE LIMITS ABOVE!

struct Node {
  int col[TableRowLimit][TableColLimit];
  string table;
  Node *next;
};

struct Database {
  string DatabaseName;
  Node TableContainer[TableLimit];
};

Database databaseContainer[DatabaseLimit];

bool CreateNewDatabase(string name) { return true; };

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

        NEW DATABASE - creates a new database, 
    
    )";
    return Help;
  }
}

bool ExitAlias(string Text) {
  string ExitAliases[3] = {"EXIT", "BYE", "SEEYOU"};

  for (string ExitKeyword : ExitAliases) {
    if (Text == ExitKeyword) {
      return true;
    }
  }

  return false;
}

string standardizedString(string Text) {
  for (int i = 0; i < Text.length(); i++) {
    Text[i] = std::toupper(Text[i]);
  }

  return Text;
}

bool DevTool() {
  string Selection;
  string StandardizedSelection;
  int KillDevTool = 0;
  string NameOfDatabase;

  while (KillDevTool != 1) {
    cout << "Welcome to CLI of PainFlux" << endl;
    cout << "Stuck? Type 'HELP' and get help!" << endl;
    cin >> Selection;

    StandardizedSelection = standardizedString(Selection);
    if (!ExitAlias(StandardizedSelection)) {
      if (StandardizedSelection == "NEW DATABASE") {
        cout << "Name the Database: ";
        cin >> NameOfDatabase;
      }
    } else {
      cout << "Bye bye!";
      KillDevTool = 1;
      continue;
    }
  }

  return true;
};

int main() {
  // Check if DevTool is true, if true enable the debugging layer
  if (EnableDevTool == true) {
    DevTool();
  };

  return 0;
}