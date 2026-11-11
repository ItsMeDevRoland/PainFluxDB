//
// facade.hpp — ergonomic layer for front-end developers
//
// The engine core (Row.hpp, table.hpp, ...) is correct but verbose: positional
// Value arrays, makeInt()/makeString() noise, and FindColumn() on every cell
// read. This facade wraps the core so the application team can do:
//
//     RowBuilder b(students);
//     b.setInt("ID", 2026005);
//     b.setString("FIRST", "Maria");
//     b.setDouble("GPA", 1.75);
//     b.commit();
//     DisplayAllRows(students);
//     cout << CellText(students, r, "GPA") << endl;
//
// Everything here is a thin, purely additive wrapper over the existing core.
// No engine behaviour changes; nothing in the core headers is touched.
//

#pragma once
#include "Row.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <string>
using namespace std;

// ---------------------------------------------------------------------------
// Group 1 — RowBuilder: fill a row by column name, order never matters.
// ---------------------------------------------------------------------------

class RowBuilder {
public:
  explicit RowBuilder(Node *table) : table_(table) {}

  // Returns false if the column does not exist or the value type mismatches
  // the schema. Until then the builder keeps whatever it has.
  bool setInt(string col, int v);
  bool setDouble(string col, double v);
  bool setString(string col, string v);
  bool setBool(string col, bool v);

  // Insert a new row. Succeeds only when every schema column has been set.
  bool commit();

  // Apply only the columns set so far onto an existing row. Treats untouched
  // columns as "leave alone".
  bool update(Row *row);

  // How many columns have been marked as set, for diagnostics.
  int filledCount() const { return filled_; }

private:
  Node *table_;
  int filled_ = 0;
  bool set_[TableColLimit] = {};
  Value value_[TableColLimit];
};

// ---------------------------------------------------------------------------
// Group 2 — named cell access (no manual FindColumn)
// ---------------------------------------------------------------------------

// Defaults on unknown column / wrong type: "" , 0, 0.0, false respectively.
string CellText(Node *table, Row *row, string col);
int CellInt(Node *table, Row *row, string col);
double CellDouble(Node *table, Row *row, string col);
bool CellBool(Node *table, Row *row, string col);

// True if the column exists and the write happened.
bool SetCell(Node *table, Row *row, string col, const Value &v);

// ---------------------------------------------------------------------------
// Group 3 — typed find / delete (no makeInt boilerplate)
// ---------------------------------------------------------------------------

Row *FindRowInt(Node *table, string col, int v);
Row *FindRowDouble(Node *table, string col, double v);
Row *FindRowStr(Node *table, string col, string v);
Row *FindRowBool(Node *table, string col, bool v);

bool DeleteRowInt(Node *table, string col, int v);
bool DeleteRowDouble(Node *table, string col, double v);
bool DeleteRowStr(Node *table, string col, string v);
bool DeleteRowBool(Node *table, string col, bool v);

// ---------------------------------------------------------------------------
// Group 4 — one-call table setup
// ---------------------------------------------------------------------------

struct TableSpec {
  string name;
  ColumnType type;
};

// Create the table (if missing) and add every column defined in `spec`.
// Returns false only if something is wrong (bad db, duplicate column, ...).
bool SetupTable(Database *db, string tableName, const TableSpec *spec,
                int specCount);

// ---------------------------------------------------------------------------
// Group 5 — schema-driven printing (header comes from the schema)
// ---------------------------------------------------------------------------

// Prints header + every row in the table.
void DisplayAllRows(Node *table);

// Prints header + the given rows (a search/sort result, for example).
void DisplayRows(Node *table, Row *const *rows, int count);

// Prints a single row (with header names as the first line). For one-offs.
void DisplayRow(Node *table, Row *row);

// ---------------------------------------------------------------------------
// Group 6 — bulk / convenience counters
// ---------------------------------------------------------------------------

// Recursive row count of a table without poking at RowHead yourself.
int CountRowsOf(Node *table);

bool IsEmpty(Node *table);
