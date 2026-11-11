// Ergonomic convenience layer on top of the engine core.
// See facade.hpp for why this exists and what it adds.

#include "facade.hpp"
#include "config.hpp"
#include "search.hpp"
#include <iostream>
#include <string>
using namespace std;

// ---------------------------------------------------------------------------
// Group 1 — RowBuilder
// ---------------------------------------------------------------------------

// Setting a value by its column index. Returns false when the column does not
// exist or the value type does not match the schema, so a wrong call cannot
// silently corrupt a row.
bool RowBuilder::setInt(string col, int v) {
  if (table_ == nullptr)
    return false;
  int index = FindColumn(table_, col);
  if (index == -1 || table_->Columns[index].Type != COL_INT)
    return false;
  value_[index] = makeInt(v);
  if (!set_[index])
    filled_ += 1;
  set_[index] = true;
  return true;
}

bool RowBuilder::setDouble(string col, double v) {
  if (table_ == nullptr)
    return false;
  int index = FindColumn(table_, col);
  if (index == -1 || table_->Columns[index].Type != COL_DOUBLE)
    return false;
  value_[index] = makeDouble(v);
  if (!set_[index])
    filled_ += 1;
  set_[index] = true;
  return true;
}

bool RowBuilder::setString(string col, string v) {
  if (table_ == nullptr)
    return false;
  int index = FindColumn(table_, col);
  if (index == -1 || table_->Columns[index].Type != COL_STRING)
    return false;
  value_[index] = makeString(v);
  if (!set_[index])
    filled_ += 1;
  set_[index] = true;
  return true;
}

bool RowBuilder::setBool(string col, bool v) {
  if (table_ == nullptr)
    return false;
  int index = FindColumn(table_, col);
  if (index == -1 || table_->Columns[index].Type != COL_BOOL)
    return false;
  value_[index] = makeBool(v);
  if (!set_[index])
    filled_ += 1;
  set_[index] = true;
  return true;
}

bool RowBuilder::commit() {
  if (table_ == nullptr)
    return false;

  // A full row must cover the whole schema, otherwise InsertRow would reject
  // it. Requiring completion here catches mistakes at the call site.
  if (filled_ != table_->ColumnCount)
    return false;

  Value row[TableColLimit];
  for (int i = 0; i < table_->ColumnCount; i++) {
    row[i] = value_[i];
  }
  return InsertRow(table_, row, table_->ColumnCount);
}

bool RowBuilder::update(Row *row) {
  if (table_ == nullptr || row == nullptr || filled_ == 0)
    return false;

  bool any = false;
  for (int i = 0; i < table_->ColumnCount; i++) {
    if (set_[i]) {
      if (!UpdateRow(table_, row, table_->Columns[i].ColName, value_[i]))
        return false;
      any = true;
    }
  }
  return any;
}

// ---------------------------------------------------------------------------
// Group 2 — named cell access
// ---------------------------------------------------------------------------

static const Value *cellAt(Node *table, Row *row, string col) {
  if (table == nullptr || row == nullptr)
    return nullptr;
  int index = FindColumn(table, col);
  if (index == -1 || index >= row->ColumnCount)
    return nullptr;
  return &row->Values[index];
}

string CellText(Node *table, Row *row, string col) {
  const Value *v = cellAt(table, row, col);
  if (v == nullptr)
    return "";
  if (v->Type == COL_INT)
    return to_string(v->Int);
  return valueToString(*v);
}

int CellInt(Node *table, Row *row, string col) {
  const Value *v = cellAt(table, row, col);
  if (v == nullptr)
    return 0;
  if (v->Type == COL_INT)
    return v->Int;
  return static_cast<int>(v->Num);
}

double CellDouble(Node *table, Row *row, string col) {
  const Value *v = cellAt(table, row, col);
  if (v == nullptr)
    return 0.0;
  if (v->Type == COL_DOUBLE)
    return v->Num;
  return static_cast<double>(v->Int);
}

bool CellBool(Node *table, Row *row, string col) {
  const Value *v = cellAt(table, row, col);
  if (v == nullptr)
    return false;
  if (v->Type == COL_BOOL)
    return v->Bool;
  return false;
}

bool SetCell(Node *table, Row *row, string col, const Value &v) {
  if (table == nullptr || row == nullptr)
    return false;
  return UpdateRow(table, row, col, v);
}

// ---------------------------------------------------------------------------
// Group 3 — typed find / delete
// ---------------------------------------------------------------------------

Row *FindRowInt(Node *table, string col, int v) {
  return FindRow(table, col, makeInt(v));
}
Row *FindRowDouble(Node *table, string col, double v) {
  return FindRow(table, col, makeDouble(v));
}
Row *FindRowStr(Node *table, string col, string v) {
  return FindRow(table, col, makeString(v));
}
Row *FindRowBool(Node *table, string col, bool v) {
  return FindRow(table, col, makeBool(v));
}

bool DeleteRowInt(Node *table, string col, int v) {
  return DeleteRow(table, col, makeInt(v));
}
bool DeleteRowDouble(Node *table, string col, double v) {
  return DeleteRow(table, col, makeDouble(v));
}
bool DeleteRowStr(Node *table, string col, string v) {
  return DeleteRow(table, col, makeString(v));
}
bool DeleteRowBool(Node *table, string col, bool v) {
  return DeleteRow(table, col, makeBool(v));
}

// ---------------------------------------------------------------------------
// Group 4 — one-call table setup
// ---------------------------------------------------------------------------

bool SetupTable(Database *db, string tableName, const TableSpec *spec,
                int specCount) {
  if (db == nullptr || spec == nullptr || specCount < 0)
    return false;

  // Create only when missing so SetupTable is idempotent (safe to call on
  // startup every run).
  if (SearchTable(db, tableName) == nullptr) {
    if (!CreateTable(db, tableName))
      return false;
  }

  Node *table = SearchTable(db, tableName);
  if (table == nullptr)
    return false;

  for (int i = 0; i < specCount; i++) {
    int existing = FindColumn(table, spec[i].name);
    if (existing != -1) {
      // Already there. Accept it only if the type matches, so a second call
      // is a no-op but a contradictory schema is still reported.
      if (table->Columns[existing].Type != spec[i].type) {
        return false;
      }
      continue;
    }

    if (!AddColumn(table, spec[i].name, spec[i].type)) {
      return false; // column limit reached or duplicate
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
// Group 5 — schema-driven printing
// ---------------------------------------------------------------------------

static void printHeader(Node *table) {
  for (int i = 0; i < table->ColumnCount; i++) {
    if (i > 0)
      cout << " | ";
    cout << table->Columns[i].ColName;
  }
  cout << endl;
}

static void printOne(Node *table, Row *row) {
  for (int i = 0; i < table->ColumnCount; i++) {
    if (i > 0)
      cout << " | ";
    cout << valueToString(row->Values[i]);
  }
  cout << endl;
}

void DisplayAllRows(Node *table) {
  if (table == nullptr)
    return;
  printHeader(table);
  Row *current = table->RowHead;
  while (current != nullptr) {
    printOne(table, current);
    current = current->next;
  }
}

void DisplayRows(Node *table, Row *const *rows, int count) {
  if (table == nullptr || rows == nullptr)
    return;
  printHeader(table);
  for (int i = 0; i < count; i++) {
    printOne(table, rows[i]);
  }
}

void DisplayRow(Node *table, Row *row) {
  if (table == nullptr || row == nullptr)
    return;
  printHeader(table);
  printOne(table, row);
}

// ---------------------------------------------------------------------------
// Group 6 — bulk / convenience counters
// ---------------------------------------------------------------------------

int CountRowsOf(Node *table) {
  if (table == nullptr)
    return 0;
  return CountRows(table->RowHead);
}

bool IsEmpty(Node *table) {
  if (table == nullptr)
    return true;
  return table->RowHead == nullptr;
}
