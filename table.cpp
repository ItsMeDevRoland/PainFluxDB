// THIS IS THE TABLE FUNCTIONS, DO NOT TOUCH, THIS HAS NO CONFIGS, VISIT
// CONFIG.HPP FOR CONFIGURATION
#include "table.hpp"
#include "struct.hpp"
#include <iostream>
#include <string>
using namespace std;

bool CreateTable(Database *db, string tableName) {

  if (db == nullptr) {
    return false;
  }

  // Refuse duplicates so SearchTable always has exactly one answer.
  if (SearchTable(db, tableName) != nullptr) {
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

void DisplayTables(Database *db, bool EnablePFdbDefaults) {

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
    while (Current != nullptr) {
      cout << Current->table << " (" << Current->RowCount << " rows)" << endl;
      Current = Current->next;
    }
  }
}

Node *SearchTable(Database *db, string tableName) {

  if (db == nullptr) {
    return nullptr;
  }

  Node *Current = db->TableHead;
  while (Current != nullptr) {
    if (tableName == Current->table) {
      return Current;
    }
    Current = Current->next;
  }

  return nullptr;
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

    // The table owns rows, stack nodes and queue nodes. Free all three before
    // the table itself, otherwise deleting a table leaks every record in it.
    Row *row = Table->RowHead;
    while (row != nullptr) {
      Row *nextRow = row->next;
      delete row;
      row = nextRow;
    }

    StackNode *stackNode = Table->StackTop;
    while (stackNode != nullptr) {
      StackNode *nextStack = stackNode->next;
      delete stackNode->row; // stack owns rows it removed from the table
      delete stackNode;
      stackNode = nextStack;
    }

    QueueNode *queueNode = Table->QueueFront;
    while (queueNode != nullptr) {
      QueueNode *nextQueue = queueNode->next;
      // The queue only borrows rows; they are freed with the row chain above.
      delete queueNode;
      queueNode = nextQueue;
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

bool AddColumn(Node *table, string columnName, ColumnType type) {
  if (table == nullptr) {
    return false;
  }

  if (table->ColumnCount >= TableColLimit) {
    return false;
  }

  // Duplicate column names would make FindColumn ambiguous.
  if (FindColumn(table, columnName) != -1) {
    return false;
  }

  table->Columns[table->ColumnCount].ColName = columnName;
  table->Columns[table->ColumnCount].Type = type;
  table->ColumnCount += 1;
  return true;
}

int FindColumn(Node *table, string columnName) {
  if (table == nullptr) {
    return -1;
  }

  for (int i = 0; i < table->ColumnCount; i++) {
    if (table->Columns[i].ColName == columnName) {
      return i;
    }
  }
  return -1;
}
