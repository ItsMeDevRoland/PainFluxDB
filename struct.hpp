// Struct libraries, This is where the struct are placed
#pragma once

#include "config.hpp"
#include <string>
using namespace std;

// A column can hold one of these types. The engine is generic: it does not
// know or care what the data means, only how to store and compare it.
enum ColumnType { COL_INT, COL_DOUBLE, COL_STRING, COL_BOOL };

// One cell. A tagged struct instead of a union keeps the code readable and
// the course cares more about clarity than shaving 24 bytes.
struct Value {
  ColumnType Type = COL_STRING;
  string Str;
  double Num = 0.0;
  int Int = 0;
  bool Bool = false;
};

// Schema entry: a name and what kind of value lives in that position.
struct Column {
  string ColName;
  ColumnType Type = COL_STRING;
};

// A row is a linked-list node. Values are positional and line up with the
// owning table's Columns[] array.
struct Row {
  Value Values[TableColLimit];
  int ColumnCount = 0;
  Row *next = nullptr;
};

// Stack node for recently deleted rows (LIFO).
struct StackNode {
  Row *row = nullptr;
  StackNode *next = nullptr;
};

// Queue node for pending rows (FIFO).
struct QueueNode {
  Row *row = nullptr;
  QueueNode *next = nullptr;
};

struct Database;

// A table. Also a linked-list node: databases chain their tables through
// `next`. Each table owns its rows, its deleted-row stack and its pending queue.
struct Node {
  string table;
  Column Columns[TableColLimit];
  int ColumnCount = 0;

  Row *RowHead = nullptr;
  Row *RowTail = nullptr;
  int RowCount = 0;

  Database *DatabaseParent = nullptr;
  Node *next = nullptr;

  StackNode *StackTop = nullptr;
  int StackCount = 0;

  QueueNode *QueueFront = nullptr;
  QueueNode *QueueRear = nullptr;
  int QueueCount = 0;
};

struct Database {
  string DatabaseName;
  Node *TableHead = nullptr;
};

struct Commands {
  string CommandName;
  string CommandAliases[MaxAliasSize];
};

// CONTAINERS: fixed-size buckets used to hand lists back to the caller without
// allocating. The caller reads up to `DatabaseCount` entries.
struct Containers {
  int IntArray[MaxArrayContainer];
  string StringArray[MaxArrayContainer];
  float FloatArray[MaxArrayContainer];
  Database *DatabaseArray[MaxArrayContainer];
  int DatabaseCount;
};
