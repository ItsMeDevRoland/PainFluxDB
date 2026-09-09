// Struct libraries, This is where the struct are placed
#pragma once

#include "config.hpp"
#include <string>
using namespace std;

struct Database;

struct Node {
  int col[TableRowLimit][TableColLimit];
  string table;
  Database* DatabaseParent;
  Node *next;
};

struct Database {
  string DatabaseName;
  Node TableContainer[TableLimit];
};

struct Commands {
  string CommandName;
  string CommandAliases[MaxAliasSize];
};

struct Containers {
    int IntArray[MaxArrayContainer];
    string StringArray[MaxArrayContainer];
    float FloatArray[MaxArrayContainer];
    Database* DatabaseArray[MaxArrayContainer];
};
