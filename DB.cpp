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
#include "table.hpp"
#include "utility.hpp"
#include <iostream>
#include <string>
using namespace std;

// VARIABLES USED FOR GLOBAL //
Database databaseContainer[DatabaseLimit];

Database *DatabaseSelected = nullptr;

Containers ArrayContainer;

// OBTAIN THE SIZE OF THE DATABASE //
// NOTE: this returns a COUNT of used slots, it is NOT a valid write index.
// Use findFreeDBSlot() when you need somewhere to put a new database.
int getDBSize() {
  int DatabaseMax = count_Array(databaseContainer, DatabaseLimit);
  return DatabaseMax;
}

// FINDS THE FIRST EMPTY SLOT //
// Arrays are not packed together, deleting a database leaves a hole.
// So we scan for the first free slot instead of assuming count == index.
int findFreeDBSlot() {
  for (int slot = 0; slot < DatabaseLimit; slot++) {
    if (databaseContainer[slot].DatabaseName.empty()) {
      return slot;
    }
  }
  return -1; // No free slot, database limit reached
}

// Creates Database w/o input system //
bool headlessCreateNewDatabase(string name) {
  // Reject duplicate names, otherwise we would have two slots with the
  // same name and only one of them could ever be found again
  if (returnQueryADB(name) != nullptr) {
    return false;
  }

  int FreeSlot = findFreeDBSlot();
  if (FreeSlot == -1) {
    return false; // Out of database slots
  }

  databaseContainer[FreeSlot].DatabaseName = name;

  return true;
}

// Create Database with input System //
bool CreateNewDatabase(string MessageOverride) {
  string DatabaseName;

  // Check if the Override exist //
  if (MessageOverride.empty()) {
    cout << "Create The name of your database: ";
  } else {
    // If Yes, Show it instead of our Default //
    cout << MessageOverride << endl;
  };

  // Guard against EOF/failed input
  if (!(cin >> DatabaseName)) {
    cout << "[ERROR] No database name given" << endl;
    return false;
  }

  // Call the headless New Database
  bool DatabaseOperation = headlessCreateNewDatabase(DatabaseName);

  // Responce, if it went TRUE, then it worked, else RETURN FALSE
  if (DatabaseOperation) {
    return true;
  }
  return false;
};

// Return A Query A DATABASE, returns only one and returns the DATABASE ITSELF
// NOTE: returns a POINTER, not a copy. A Database is 4MB, copying it around
// would eat the stack.
Database *returnQueryADB(string Name) {
  if (Name.empty()) {
    return &databaseContainer[0];
  };

  // Scan the FULL range, not getDBSize(). getDBSize() is a count, so a hole
  // left by a deleted database would hide every database behind it.
  for (int Database = 0; Database < DatabaseLimit; Database++) {
    if (databaseContainer[Database].DatabaseName == Name) {
      return &databaseContainer[Database];
    }
  };

  return nullptr;
}

// Returns A Query with ONLY NAME ONLY, not the database
string returnNameQueryADB(string Name) {
  Database *DatabaseQueried = returnQueryADB(Name);
  if (DatabaseQueried == nullptr) {
    return "";
  }
  return DatabaseQueried->DatabaseName;
}

// Returns a Container of ARRAY who contains all the Queried DB name
Containers returnAllDB(string Name) {
  // Clear first, otherwise a filtered query leaves stale pointers behind
  clearArrayContainer();

  int ArrayCounter = 0;

  for (int Database = 0; Database < DatabaseLimit; Database++) {
    if (databaseContainer[Database].DatabaseName.empty()) {
      continue; // Skip the holes left by deleted databases
    }

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

  ArrayContainer.DatabaseCount = ArrayCounter;

  return ArrayContainer;
}

// SHOWS EVERY DATABASE STORED IN THE ENGINE //
// NOTE: this lives in DB.cpp, not in the CLI file, so the engine can be
// embedded as a library without dragging its own main() along.
void showAllDB() {
  cout << " || ALL DATABASE STORED ||" << endl;

  Containers DatabaseList = returnAllDB();
  // Use DatabaseCount, not getDBSize(), because the container only holds the
  // databases that actually exist
  for (int i = 0; i < DatabaseList.DatabaseCount; i++) {
    cout << " || " << DatabaseList.DatabaseArray[i]->DatabaseName << " ||"
         << endl;
  }
}

// Headless Use DB, It will place an insert without showing any texts
// NOTE: returns true if a database was actually selected
bool headlessUseDB(string Name) {
  if (Name.empty()) {
    DatabaseSelected = nullptr;
    return true;
  }

  for (int Database = 0; Database < DatabaseLimit; Database++) {
    if (databaseContainer[Database].DatabaseName.empty()) {
      continue; // Skip holes
    }

    if (Name == databaseContainer[Database].DatabaseName) {
      DatabaseSelected = &databaseContainer[Database];
      return true; // Stop at the first match
    }
  }

  // No match, the requested database does not exist
  DatabaseSelected = nullptr;
  return false;
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

  // Guard against EOF/failed input
  if (!(cin >> ToUseDatabase)) {
    cout << "[ERROR] No database name given" << endl;
    DatabaseSelected = nullptr;
    return;
  }

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
    return;
  }

  bool Found = headlessUseDB(ToUseDatabase);

  if (Found) {
    // Display custom Success if it contains some
    if (SuccessOverride.empty()) {
      cout << "[SUCCESS] Using " << DatabaseSelected->DatabaseName << endl;
    } else {
      cout << SuccessOverride << endl;
    }
  } else {
    // The old code just did "continue" and said nothing, so a typo was
    // completely silent. Now we actually complain.
    if (EnableDevTool) {
      if (ErrorOverride.empty()) {
        cout << "[ERROR] No database named " << ToUseDatabase << endl;
      } else {
        cout << ErrorOverride << endl;
      }
    }
  }
}

bool headlessDeleteDatabase(string Name) {
  for (int Database = 0; Database < DatabaseLimit; Database++) {
    if (databaseContainer[Database].DatabaseName.empty()) {
      continue; // Skip holes
    }

    if (Name == databaseContainer[Database].DatabaseName) {
      // Free the table chain first. Clearing only the name would leave
      // TableHead pointing at live nodes, so a reused slot would inherit the
      // old database's tables.
      DeleteTableRecursv(&databaseContainer[Database]);
      databaseContainer[Database].DatabaseName = "";

      // If the deleted database was the selected one, drop the pointer
      // instead of leaving it dangling at an empty slot
      if (DatabaseSelected == &databaseContainer[Database]) {
        DatabaseSelected = nullptr;
      }

      return true;
    }
  }

  return false;
}

void DeleteDatabase(string InputOverride, string ErrorOverride,
                    string SuccessOverride) {
  string DBtoDelete;

  if (InputOverride.empty()) {
    cout << "Type the database You wish to delete: ";
  } else {
    cout << InputOverride << endl;
  }
  if (!(cin >> DBtoDelete)) {
    cout << "[ERROR] No database name given" << endl;
    return;
  }

  bool DeleteOperation = headlessDeleteDatabase(DBtoDelete);
  if (DeleteOperation) {
    if (SuccessOverride.empty()) {
      cout << "[SUCCESS] Deleted the database" << endl;
    } else {
      cout << SuccessOverride << endl;
    }
  } else {
    if (ErrorOverride.empty()) {
      cout << "[ERROR] Failed to delete the database" << endl;
    } else {
      cout << ErrorOverride << endl;
    }
  }
}
