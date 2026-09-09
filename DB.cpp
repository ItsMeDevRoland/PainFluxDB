/*
 *
 *  Welcome to the DB.cpp
 *  This is where the DB magic happens!
 *  There is nothing to touch here, Use config.hpp for configurations
 *
 */

#include "main.hpp"
#include "utility.hpp"
#include "DB.hpp"
#include <iostream>
#include <string>
using namespace std;


int getDBSize() {
  int DatabaseMax = count_Array(databaseContainer, DatabaseLimit);
  return DatabaseMax;
}

bool headlessCreateNewDatabase(string name){

}

bool CreateNewDatabase(string MessageOverride) {
  string DatabaseName;
  if (MessageOverride == "" || MessageOverride == " ") {
    cout << "Create The name of your database: ";
    cin >> DatabaseName;
  };

  int DatabaseSize = count_Array(databaseContainer, DatabaseLimit);
  databaseContainer[DatabaseSize].DatabaseName = DatabaseName;

  cout << "Successfully created database! "
       << databaseContainer[DatabaseSize].DatabaseName << endl;

  return true;
};

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

string returnNameQueryADB(string Name) {
  Database DatabaseQueried = returnQueryADB(Name);
  return DatabaseQueried.DatabaseName;
}

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
