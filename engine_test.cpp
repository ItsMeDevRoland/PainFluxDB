//
// engine_test.cpp — self-checking test suite for PainFluxDB.
//
// Build (one line):
//   g++ -std=c++17 -Wall -Wextra engine_test.cpp DB.cpp table.cpp Row.cpp
//       stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp -o test
// Run:
//   ./test
//
// This is a deliverable too: it doubles as the project's test-case table.
//

#include "DB.hpp"
#include "Row.hpp"
#include "config.hpp"
#include "queue.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "stack.hpp"
#include "stats.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"

#include <cmath>
#include <iostream>
#include <string>
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

static void section(const string &name) { cout << "\n== " << name << " ==" << endl; }

// Builds a table with three columns: ID(int), NAME(string), SCORE(double).
static Node *makeStudentTable(const string &dbName, const string &tableName) {
  headlessDeleteDatabase(dbName);
  headlessCreateNewDatabase(dbName);
  Database *db = returnQueryADB(dbName);
  CreateTable(db, tableName);
  Node *table = SearchTable(db, tableName);
  AddColumn(table, "ID", COL_INT);
  AddColumn(table, "NAME", COL_STRING);
  AddColumn(table, "SCORE", COL_DOUBLE);
  return table;
}

static bool insertStudent(Node *table, int id, string name, double score) {
  Value row[3];
  row[0] = makeInt(id);
  row[1] = makeString(name);
  row[2] = makeDouble(score);
  return InsertRow(table, row, 3);
}

int main() {
  cout << "PainFluxDB engine test suite" << endl;

  // ---------------------------------------------------------------- schema
  section("Schema");
  Node *t = makeStudentTable("TESTDB", "STUDENTS");
  check(t != nullptr, "table created");
  check(t->ColumnCount == 3, "3 columns registered");
  check(FindColumn(t, "NAME") == 1, "FindColumn(NAME) == 1");
  check(FindColumn(t, "MISSING") == -1, "FindColumn(missing) == -1");
  check(!AddColumn(t, "ID", COL_INT), "duplicate column rejected");
  check(!CreateTable(returnQueryADB("TESTDB"), "STUDENTS"),
        "duplicate table rejected");

  // ------------------------------------------------------------- insertion
  section("Insertion");
  check(insertStudent(t, 1001, "Cruz", 1.75), "insert 1001");
  check(insertStudent(t, 1003, "Santos", 1.50), "insert 1003");
  check(insertStudent(t, 1002, "Reyes", 1.90), "insert 1002");
  check(t->RowCount == 3, "RowCount == 3");
  check(CountRows(t->RowHead) == 3, "recursive CountRows == 3");
  check(t->RowTail != nullptr && t->RowTail->Values[0].Int == 1002,
        "tail points at last inserted row");

  Value wrongShape[2];
  check(!InsertRow(t, wrongShape, 2), "row with wrong column count rejected");

  // ---------------------------------------------------------------- search
  section("Searching");
  Row *found[MaxRowReturn];
  int n = LinearSearch(t, "ID", makeInt(1003), found, MaxRowReturn);
  check(n == 1 && found[0]->Values[1].Str == "Santos", "linear search finds 1003");

  n = BinarySearch(t, "ID", makeInt(1002), found, MaxRowReturn);
  check(n == 1 && found[0]->Values[1].Str == "Reyes", "binary search finds 1002");

  n = LinearSearch(t, "ID", makeInt(9999), found, MaxRowReturn);
  check(n == 0, "linear search misses unknown id");
  n = BinarySearch(t, "ID", makeInt(9999), found, MaxRowReturn);
  check(n == 0, "binary search misses unknown id");
  n = BinarySearch(t, "NOPE", makeInt(1), found, MaxRowReturn);
  check(n == 0, "search on unknown column returns 0");

  // ---------------------------------------------------------------- sorting
  section("Sorting");
  check(SortRows(t, "ID", BUBBLE, true), "bubble sort by ID asc");
  check(t->RowHead->Values[0].Int == 1001 &&
            t->RowHead->next->Values[0].Int == 1002 &&
            t->RowHead->next->next->Values[0].Int == 1003,
        "bubble order 1001,1002,1003");

  check(SortRows(t, "ID", SELECTION, false), "selection sort by ID desc");
  check(t->RowHead->Values[0].Int == 1003, "desc head is 1003");
  check(t->RowTail->Values[0].Int == 1001, "desc tail is 1001");

  check(SortRows(t, "SCORE", INSERTION, false), "insertion sort by SCORE desc");
  check(t->RowHead->Values[2].Num > 1.89 &&
            t->RowHead->Values[2].Num < 1.91,
        "highest score first");

  check(SortRows(t, "NAME", BUBBLE, true), "bubble sort by NAME asc");
  check(t->RowHead->Values[1].Str == "Cruz" &&
            t->RowHead->next->Values[1].Str == "Reyes" &&
            t->RowHead->next->next->Values[1].Str == "Santos",
        "alphabetical order Cruz,Reyes,Santos");

  check(!SortRows(t, "NOPE", BUBBLE, true), "sort on unknown column fails");

  // ----------------------------------------------------------------- update
  section("Update");
  Row *target = FindRow(t, "ID", makeInt(1002));
  check(target != nullptr, "FindRow finds 1002");
  check(UpdateRow(t, target, "SCORE", makeDouble(1.25)), "update score");
  check(FindRow(t, "ID", makeInt(1002))->Values[2].Num > 1.24 &&
            FindRow(t, "ID", makeInt(1002))->Values[2].Num < 1.26,
        "score persisted");
  check(!UpdateRow(t, target, "NOPE", makeInt(1)), "update unknown column fails");

  // ------------------------------------------------------------- stack LIFO
  section("Stack (deleted rows)");
  check(t->StackCount == 0, "stack starts empty");
  check(DeleteRow(t, "ID", makeInt(1001)), "delete 1001");
  check(DeleteRow(t, "ID", makeInt(1003)), "delete 1003");
  check(t->RowCount == 1, "one row left");
  check(t->StackCount == 2, "two rows on the stack");
  check(PeekDeleted(t)->Values[0].Int == 1003, "peek returns newest (1003)");

  Row *stackRows[MaxRowReturn];
  int stackCount = DisplayStack(t, stackRows, MaxRowReturn);
  check(stackCount == 2, "DisplayStack returns 2");
  check(stackRows[0]->Values[0].Int == 1003 &&
            stackRows[1]->Values[0].Int == 1001,
        "stack order newest-first (1003,1001)");

  Row *popped = PopDeleted(t);
  check(popped != nullptr && popped->Values[0].Int == 1003, "pop returns 1003");
  check(t->StackCount == 1, "stack count drops to 1");
  delete popped; // test owns it now
  check(!DeleteRow(t, "ID", makeInt(4242)), "deleting missing row fails");

  // Undo delete: restore the remaining deleted row back into the table.
  check(RestoreDeleted(t), "restore deleted row");
  check(t->StackCount == 0, "stack empty after restore");
  check(t->RowCount == 2, "row count back to 2");
  check(FindRow(t, "ID", makeInt(1001)) != nullptr, "restored row is 1001");
  check(!RestoreDeleted(t), "restore on empty stack fails");

  // ------------------------------------------------------------- queue FIFO
  section("Queue (service)");
  Node *q = makeStudentTable("QUEUEDB", "WAITING");
  insertStudent(q, 2001, "Juan", 2.00);
  insertStudent(q, 2002, "Maria", 1.80);
  insertStudent(q, 2003, "Pedro", 1.60);

  Row *juan = FindRow(q, "ID", makeInt(2001));
  Row *maria = FindRow(q, "ID", makeInt(2002));
  Row *pedro = FindRow(q, "ID", makeInt(2003));

  check(Enqueue(q, juan), "enqueue Juan");
  check(Enqueue(q, maria), "enqueue Maria");
  check(Enqueue(q, pedro), "enqueue Pedro");
  check(q->QueueCount == 3, "queue holds 3");
  check(PeekQueue(q) == juan, "peek is first in (Juan)");

  Row *queueRows[MaxRowReturn];
  int queueCount = DisplayQueue(q, queueRows, MaxRowReturn);
  check(queueCount == 3, "DisplayQueue returns 3");
  check(queueRows[0] == juan && queueRows[1] == maria && queueRows[2] == pedro,
        "queue order FIFO Juan,Maria,Pedro");

  check(Dequeue(q) == juan, "dequeue returns Juan");
  check(q->QueueCount == 2, "queue count drops to 2");
  check(Dequeue(q) == maria, "dequeue returns Maria");
  check(Dequeue(q) == pedro, "dequeue returns Pedro");
  check(Dequeue(q) == nullptr, "dequeue on empty returns null");
  check(q->QueueRear == nullptr, "rear reset after empty");

  // Deleting a queued row must remove it from the queue too.
  section("Queue + delete interaction");
  Enqueue(q, FindRow(q, "ID", makeInt(2001)));
  check(q->QueueCount == 1, "row queued again");
  DeleteRow(q, "ID", makeInt(2001));
  check(q->QueueCount == 0, "delete removes row from queue");

  // ------------------------------------------------------------ statistics
  section("Statistics");
  Node *s = makeStudentTable("STATSDB", "SCORES");
  insertStudent(s, 1, "A", 1.00);
  insertStudent(s, 2, "B", 2.00);
  insertStudent(s, 3, "C", 3.00);

  ColumnStats stats = ComputeStats(s, "SCORE");
  check(stats.count == 3, "stats count == 3");
  check(fabs(stats.min - 1.0) < 1e-9, "stats min == 1.0");
  check(fabs(stats.max - 3.0) < 1e-9, "stats max == 3.0");
  check(fabs(stats.sum - 6.0) < 1e-9, "stats sum == 6.0");
  check(fabs(stats.avg - 2.0) < 1e-9, "stats avg == 2.0");

  ColumnStats textStats = ComputeStats(s, "NAME");
  check(textStats.count == 0, "stats on text column refused");

  ColumnStats emptyStats = ComputeStats(s, "NOPE");
  check(emptyStats.count == 0, "stats on unknown column empty");

  // ------------------------------------------------------- lifecycle/leaks
  section("Lifecycle");
  check(headlessDeleteDatabase("TESTDB"), "delete TESTDB");
  check(returnQueryADB("TESTDB") == nullptr, "TESTDB gone");
  headlessCreateNewDatabase("TESTDB");
  Database *reused = returnQueryADB("TESTDB");
  check(reused != nullptr && reused->TableHead == nullptr,
        "reused slot has no ghost tables");

  headlessDeleteDatabase("QUEUEDB");
  headlessDeleteDatabase("STATSDB");
  headlessDeleteDatabase("TESTDB");

  // ---------------------------------------------------------------- memory
  section("Memory footprint");
  cout << "  sizeof(Row)        = " << sizeof(Row) << " bytes" << endl;
  cout << "  sizeof(Value)      = " << sizeof(Value) << " bytes" << endl;
  cout << "  sizeof(Node/Table) = " << sizeof(Node) << " bytes" << endl;
  check(sizeof(Row) < 20000, "Row stays small (was 2.98MB per row)");
  check(sizeof(Node) < 100000, "Table node stays small (was 297MB)");

  cout << "\n----------------------------------------" << endl;
  cout << "PASSED: " << passed << "   FAILED: " << failed << endl;
  cout << "----------------------------------------" << endl;
  return failed == 0 ? 0 : 1;
}
