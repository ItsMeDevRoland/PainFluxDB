// THIS IS THE TABLE FUNCTIONS, DO NOT TOUCH, THIS HAS NO CONFIGS, VISIT
// CONFIG.HPP FOR CONFIGURATION
#include "struct.hpp"
#include <iostream>
#include <string>
using namespace std;

bool CreateTable(Database *db, string tableName) {

  if (db == nullptr) {
    return false;
  }

  Node *Table = new Node;
  Table->DatabaseParent = db;
  Table->next = nullptr;
  Table->table = tableName;

  if (db->TableHead == nullptr) {
    db->TableHead = Table;
  } else {
    Node *Current = db->TableHead;

    while (Current->next != nullptr) {
      Current = Current->next;
    }

    Current->next = Table;
  }

  return true;
}

void DisplayTables(Database *db, bool EnablePFdbDefaults = true) {

  if (db == nullptr) {
    return;
  }

  if (db->TableHead == nullptr) {
    if (EnablePFdbDefaults == true) {
      cout << "ERROR INVALID OPERATIONS";
    }
  } else {
    Node *Current = db->TableHead;
    cout << "TABLES: " << endl;
    do {
      cout << Current->table;
      Current = Current->next;
    } while (Current != nullptr);
  }
}

Node *SearchTable(Database *db, string tableName) {

  if (db == nullptr) {
    return nullptr;
  }

  Node *Current = db->TableHead;
  do {
    if (Current == nullptr) {
      return nullptr;
    }

    if (tableName == Current->table) {
      break;
    };
    Current = Current->next;
  } while (Current != nullptr);

  return Current;
};

bool DeleteTable(Database *db, string tableNameToDelete) {

  if (db == nullptr) {
    return false;
  } // validation

  Node *Table = db->TableHead;

  if (Table == nullptr) {
    return false;
  }

  bool TargetFound = false;
  Node *TablePrevContainer = nullptr;

  while (TargetFound == false && Table != nullptr) {
    if (Table->table != tableNameToDelete) {
      TablePrevContainer = Table;
      Table = Table->next;
    } else {
      TargetFound = true;
      break;
    }
  };

  if (TargetFound == true) {
    if (TablePrevContainer == nullptr) {
      db->TableHead = Table->next;
    } else {
      TablePrevContainer->next = Table->next;
    }
    delete Table;
    return true;
  };
  return false;
};

bool DeleteTableRecursv(Database *db) {
  if (db == nullptr) {
    return false;
  };

  while (db->TableHead != nullptr) {
    DeleteTable(db, db->TableHead->table);
  }
  return true;
}

bool DropDatabase(Database *db) {
  if (db == nullptr) {
    return false;
  }

  DeleteTableRecursv(db);
  delete db;
  return true;
}