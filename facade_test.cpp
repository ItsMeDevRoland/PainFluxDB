//
// facade_test.cpp — convenience-layer tests (standalone)
//
// Build:
//   g++ -std=c++17 -Wall -Wextra -g facade_test.cpp facade.cpp DB.cpp
//       table.cpp Row.cpp stack.cpp queue.cpp search.cpp sort.cpp stats.cpp
//       utility.cpp -o /tmp/ftest
// Run: ./ftest
//
// The facade builds on the core engine. This suite exercises facade.hpp
// directly; the core engine is covered separately by engine_test.cpp.
//
#include "DB.hpp"
#include "facade.hpp"
#include <iostream>
using namespace std;

static int passed = 0;
static int failed = 0;

static void check(bool condition, const string &label) {
  if (condition) {
    passed += 1;
    cout << "  [PASS] " << label << endl;
  } else {
    failed += 1;
    cout << "  [FAIL] " << label << endl;
  }
}

static void section(const string &name) {
  cout << "\n== FACADE: " << name << " ==" << endl;
}

// Wipe the database name, create it fresh, and return its pointer.
// The pointer MUST be fetched after creation; fetching before can hand back a
// stale or null pointer and is the classic way to crash this engine.
static Database *freshDB(const string &name) {
  headlessDeleteDatabase(name);
  headlessCreateNewDatabase(name);
  return returnQueryADB(name);
}

// ----------------------------------------------------------------------
// Group 1 — RowBuilder
// ----------------------------------------------------------------------

static void testRowBuilder() {
  section("RowBuilder");
  Database *db = freshDB("FACADE1");
  CreateTable(db, "STUDENTS");
  Node *t = SearchTable(db, "STUDENTS");
  AddColumn(t, "ID", COL_INT);
  AddColumn(t, "NAME", COL_STRING);
  AddColumn(t, "SCORE", COL_DOUBLE);

  RowBuilder b(t);
  check(b.setInt("ID", 1), "set ID");
  check(b.setString("NAME", "Cruz"), "set NAME");
  check(b.setBool("SCORE", true) == false, "type mismatch rejected");
  check(b.filledCount() == 2, "only 2 of 3 filled");
  check(!b.commit(), "commit fails while a column is missing");

  check(b.setDouble("SCORE", 1.75), "set SCORE");
  check(b.filledCount() == 3, "3 columns filled");
  check(b.commit(), "commit works once complete");
  check(t->RowCount == 1, "row actually inserted");

  // Order of setters must not matter.
  RowBuilder b2(t);
  check(b2.setString("NAME", "Maria") && b2.setInt("ID", 3) &&
            b2.setDouble("SCORE", 2.0) && b2.commit(),
        "insert in any order works");

  RowBuilder b3(t);
  check(b3.setString("MISSING", "x") == false, "unknown column rejected");
  check(!b3.commit(), "commit with nothing set rejected");

  // update touches only the columns that were set.
  RowBuilder b4(t);
  check(b4.setDouble("SCORE", 9.9), "b4 sets score only");
  Row *m = FindRowStr(t, "NAME", "Maria");
  check(m != nullptr, "Maria exists");
  check(b4.update(m), "update existing row");
  check(CellDouble(t, m, "SCORE") == 9.9, "score updated");
  check(CellInt(t, m, "ID") == 3, "untouched column preserved");

  RowBuilder nullBuilder(nullptr);
  check(nullBuilder.setDouble("A", 1.0) == false, "null table rejected");
}

// ----------------------------------------------------------------------
// Group 2 — cell access
// ----------------------------------------------------------------------

static void testCellAccess() {
  section("Cell access");
  Database *db = freshDB("FACADE2");
  CreateTable(db, "T");
  Node *t = SearchTable(db, "T");
  AddColumn(t, "I", COL_INT);
  AddColumn(t, "D", COL_DOUBLE);
  AddColumn(t, "S", COL_STRING);
  AddColumn(t, "B", COL_BOOL);

  Value row[4];
  row[0] = makeInt(42);
  row[1] = makeDouble(1.5);
  row[2] = makeString("x");
  row[3] = makeBool(true);
  check(InsertRow(t, row, 4), "row inserted");

  Row *r = t->RowHead;
  check(r != nullptr, "row head exists");
  check(CellText(t, r, "S") == "x", "CellText reads value");
  check(CellInt(t, r, "I") == 42, "CellInt reads value");
  check(CellDouble(t, r, "D") == 1.5, "CellDouble reads value");
  check(CellBool(t, r, "B") == true, "CellBool reads value");
  check(CellBool(t, r, "I") == false, "CellBool on non-bool returns false");
  check(CellText(t, r, "NOPE") == "", "unknown col -> default");
  check(CellInt(t, nullptr, "I") == 0, "null row -> default");
  check(SetCell(t, r, "D", makeDouble(9.9)) && CellDouble(t, r, "D") == 9.9,
        "SetCell persists");
  check(SetCell(t, nullptr, "I", makeInt(1)) == false, "null row rejected");
  check(SetCell(nullptr, r, "I", makeInt(1)) == false, "null table rejected");
}

// ----------------------------------------------------------------------
// Group 3 — typed find / delete
// ----------------------------------------------------------------------

static void testTypedFindDelete() {
  section("Typed find / delete");
  Database *db = freshDB("FACADE3");
  CreateTable(db, "T");
  Node *t = SearchTable(db, "T");
  AddColumn(t, "I", COL_INT);
  AddColumn(t, "S", COL_STRING);
  AddColumn(t, "D", COL_DOUBLE);

  Value row[3];
  row[0] = makeInt(10);
  row[1] = makeString("Ana");
  row[2] = makeDouble(1.2);
  check(InsertRow(t, row, 3), "row inserted");

  check(FindRowInt(t, "I", 10) != nullptr, "FindRowInt");
  check(FindRowStr(t, "S", "Ana") != nullptr, "FindRowStr");
  check(FindRowDouble(t, "D", 1.2) != nullptr, "FindRowDouble");
  check(FindRowInt(t, "I", 99999) == nullptr, "typed find returns null");
  check(!DeleteRowInt(t, "I", 99999), "typed delete on missing fails");
  check(DeleteRowStr(t, "S", "Ana"), "DeleteRowStr works");
  check(t->RowCount == 0, "row count after delete");
  check(t->StackCount == 1, "delete pushed onto the stack");
}

// ----------------------------------------------------------------------
// Group 4 — SetupTable
// ----------------------------------------------------------------------

static void testSetupTable() {
  section("SetupTable");
  Database *db = freshDB("FACADE4");
  TableSpec spec[] = {{"ID", COL_INT},
                      {"NAME", COL_STRING},
                      {"SCORE", COL_DOUBLE}};
  check(SetupTable(db, "STUDENTS", spec, 3), "SetupTable one-shot");
  check(FindColumn(SearchTable(db, "STUDENTS"), "SCORE") == 2,
        "columns registered in order");
  check(SetupTable(db, "STUDENTS", spec, 3), "SetupTable idempotent");

  // Same name, different type: must be rejected.
  TableSpec conflicting[] = {{"ID", COL_STRING}};
  check(!SetupTable(db, "STUDENTS", conflicting, 1),
        "conflicting column type rejected");

  check(!SetupTable(nullptr, "X", spec, 3), "null db rejected");
  check(!SetupTable(db, "X", nullptr, 3), "null spec rejected");
}

// ----------------------------------------------------------------------
// Group 5 — printers (smoke: must not crash)
// ----------------------------------------------------------------------

static void testPrinters() {
  section("Printers");
  Database *db = freshDB("FACADE5");
  TableSpec spec[] = {{"ID", COL_INT},
                      {"FIRST", COL_STRING},
                      {"GPA", COL_DOUBLE}};
  SetupTable(db, "STUDENTS", spec, 3);
  Node *s = SearchTable(db, "STUDENTS");

  Value row[3];
  row[0] = makeInt(1);
  row[1] = makeString("Juan");
  row[2] = makeDouble(1.5);
  InsertRow(s, row, 3);
  row[0] = makeInt(2);
  row[1] = makeString("Maria");
  row[2] = makeDouble(2.0);
  InsertRow(s, row, 3);

  Row *rows[MaxRowReturn];
  int n = GetAllRows(s, rows, MaxRowReturn);
  check(n == 2, "GetAllRows returns 2");
  DisplayRows(s, rows, n);
  DisplayRow(s, s->RowHead);
  DisplayAllRows(s);

  // Null guards must be silent, not crashes.
  DisplayAllRows(nullptr);
  DisplayRows(nullptr, nullptr, 0);
  DisplayRow(nullptr, nullptr);
  check(true, "printers survive null input");
}

// ----------------------------------------------------------------------
// Group 6 — counters
// ----------------------------------------------------------------------

static void testCounters() {
  section("Counters");
  Database *db = freshDB("FACADE6");
  CreateTable(db, "T");
  Node *t = SearchTable(db, "T");
  AddColumn(t, "I", COL_INT);

  check(IsEmpty(t) == true, "new table is empty");
  Value row[1];
  row[0] = makeInt(1);
  InsertRow(t, row, 1);
  InsertRow(t, row, 1);

  check(CountRowsOf(t) == 2, "CountRowsOf returns 2");
  check(CountRowsOf(nullptr) == 0, "CountRowsOf null -> 0");
  check(IsEmpty(t) == false, "IsEmpty false when rows exist");
  check(IsEmpty(nullptr) == true, "IsEmpty null table");
}

// ----------------------------------------------------------------------
// Entry point
// ----------------------------------------------------------------------

int main() {
  cout << "PainFluxDB facade test suite" << endl;

  testRowBuilder();
  testCellAccess();
  testTypedFindDelete();
  testSetupTable();
  testPrinters();
  testCounters();

  headlessDeleteDatabase("FACADE1");
  headlessDeleteDatabase("FACADE2");
  headlessDeleteDatabase("FACADE3");
  headlessDeleteDatabase("FACADE4");
  headlessDeleteDatabase("FACADE5");
  headlessDeleteDatabase("FACADE6");

  cout << "\n----------------------------------------" << endl;
  cout << "PASSED: " << passed << "   FAILED: " << failed << endl;
  cout << "----------------------------------------" << endl;
  return failed == 0 ? 0 : 1;
}
