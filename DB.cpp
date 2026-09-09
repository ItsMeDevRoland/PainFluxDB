/*
 *
 *  Welcome to the DB.cpp
 *  This is where the DB magic happens!
 *  There is nothing to touch here, Use config.hpp for configurations
 *
 */

// IMPORTS //
#include "DB.hpp"
#include "config.hpp"
#include "main.hpp"
#include "struct.hpp"
#include "utility.hpp"
#include <iostream>
#include <string>
using namespace std;

// VARIABLES USED FOR GLOBAL //
Database databaseContainer[DatabaseLimit];

Database *DatabaseSelected = nullptr;

Containers ArrayContainer;

// OBTAIN THE SIZE OF THE DATABASE //
int getDBSize() {
  int DatabaseMax = count_Array(databaseContainer, DatabaseLimit);
  return DatabaseMax;
}

// Creates Database w/o input system //
bool headlessCreateNewDatabase(string name) {
  int DatabaseSize = getDBSize();
  databaseContainer[DatabaseSize].DatabaseName = name;

  return true;
}

// Create Database with input System //
bool CreateNewDatabase(string MessageOverride) {
  string DatabaseName;

  // Check if the Override exist //
  if (MessageOverride.empty()) {
    cout << "Create The name of your database: ";
    cin >> DatabaseName;
  } else {
    // If Yes, Show it instead of our Default //
    cout << MessageOverride << endl;
    cin >> DatabaseName;
  };

  // Call the headless New Database
  bool DatabaseOperation = headlessCreateNewDatabase(DatabaseName);

  // Responce, if it went TRUE, then it worked, else RETURN FALSE
  if (DatabaseOperation) {
    return true;
  }
  return false;
};

// Return A Query A DATABASE, returns only one and returns the DATABASE ITSELF
Database returnQueryADB(string Name) {
  int DatabaseSize = getDBSize();

  if (Name.empty()) {
    return databaseContainer[0];
  };

  for (int Database = 0; Database < DatabaseSize; Database++) {
    if (Name == databaseContainer[Database].DatabaseName) {
      return databaseContainer[Database];
    }
  };

  return {};
}

// Returns A Query with ONLY NAME ONLY, not the database
string returnNameQueryADB(string Name) {
  Database DatabaseQueried = returnQueryADB(Name);
  return DatabaseQueried.DatabaseName;
}

// Returns a Container of ARRAY who contains all the Queried DB name
Containers returnAllDB(string Name) {
  int DatabaseSize = getDBSize();
  int ArrayCounter = 0;

  for (int Database = 0; Database < DatabaseSize; Database++) {
    if (Name.empty()) {
      ArrayContainer.DatabaseArray[ArrayCounter] = &databaseContainer[Database];
      ArrayCounter += 1;

    } else {
      if (Name == databaseContainer[Database].DatabaseName) {
        ArrayContainer.DatabaseArray[ArrayCounter] =
            &databaseContainer[Database];
        ArrayCounter += 1;
      } else {
        continue;
      }
    }
  }

  return ArrayContainer;
}

// Headless Use DB, It will place an insert without showing any texts
void headlessUseDB(string Name) {
  for (Database &DatabaseItem : databaseContainer) {
    if (Name.empty()) {
      DatabaseSelected = nullptr;
    } else {
      if (Name == DatabaseItem.DatabaseName) {
        DatabaseSelected = &DatabaseItem;
      } else {
        continue;
      }
    }
  }
}

// Use DB, has a texts, Input Override overrides the input text, ErrorOverride
// overrides the error when it failed, SuccessOverride overrides success text
// when it succeed
void UseDB(string InputOverride, string ErrorOverride, string SuccessOverride) {
  string ToUseDatabase;

  if (InputOverride.empty()) {
    cout << "Type The Database You want to use["
         << (DatabaseSelected == nullptr ? "None"
                                         : DatabaseSelected->DatabaseName)
         << "]: ";
  } else {
    cout << InputOverride << endl;
  }
  cin >> ToUseDatabase;

  for (Database &DatabaseItem : databaseContainer) {
    if (ToUseDatabase.empty()) {
      if (EnableDevTool) {
        // Display Custom ERROR if it contains some
        if (ErrorOverride.empty()) {
          cout << "[ERROR] USE cannot have an EMPTY String Query" << endl;
        } else {
          cout << ErrorOverride << endl;
        }
      }
      DatabaseSelected = nullptr;
    } else {
      if (ToUseDatabase == DatabaseItem.DatabaseName) {
        DatabaseSelected = &DatabaseItem;
        // Display custom Success if it contains some
        if (SuccessOverride.empty()) {
          cout << "[SUCCESS] Using " << DatabaseSelected->DatabaseName << endl;
        } else {
          cout << SuccessOverride << endl;
        }
      } else {
        continue;
      }
    }
  }
}

bool headlessDeleteDatabase(string Name){
    for (Database& DatabaseItem : databaseContainer){
        if (Name == DatabaseItem.DatabaseName){
            DatabaseItem = Database{};
            return true;
        }
    }

    return false;
}

void DeleteDatabase(string InputOverride, string ErrorOverride, string SuccessOverride){
    string DBtoDelete;

    if (InputOverride.empty()){
        cout << "Type the database You wish to delete: ";
    } else {
        cout << InputOverride << endl;
    }
    cin >> DBtoDelete;

    bool DeleteOperation = headlessDeleteDatabase(DBtoDelete);
    if (DeleteOperation){
        if (SuccessOverride.empty()){
            cout << "[SUCCESS] Deleted the database" << endl;
        } else {
            cout << SuccessOverride << endl;
        }
    } else {
        if (ErrorOverride.empty()){
            cout << "[ERROR] Failed to delete the database" << endl;
        } else {
            cout << ErrorOverride << endl;
        }
    }
}
