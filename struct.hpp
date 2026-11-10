// Struct libraries, This is where the struct are placed
#pragma once

#include "config.hpp"
#include <string>
using namespace std;

template <typename T> struct Column { // Column Definitions
  string ColName;
  T Value;
  string Constraint;
};

struct RowContainer { // Row definition, per Row there should be 4 Column built
                      // per dataType
  Column<string> ColString[TableRowLimit][TableColLimit];
  Column<bool> ColBool[TableRowLimit][TableColLimit];
  Column<double> ColDouble[TableRowLimit][TableColLimit];
  Column<int> ColInt[TableRowLimit][TableColLimit];
};

struct Database;

struct Node { // table, This is where the linkedList is
  RowContainer Rows[TableRowLimit];
  string table;
  Database *DatabaseParent;
  Node *next;
};

struct Database {
  string DatabaseName;
  Node *TableHead = nullptr;
};

struct Commands {
  string CommandName;
  string CommandAliases[MaxAliasSize];
};

// CONTAINERS, DOESNT DO WHAT IT IS SAID, i forgot what it do, but its for
// containing something!
struct Containers {
  int IntArray[MaxArrayContainer];
  string StringArray[MaxArrayContainer];
  float FloatArray[MaxArrayContainer];
  Database *DatabaseArray[MaxArrayContainer];
  int DatabaseCount; // How many DatabaseArray slots are actually filled
};
