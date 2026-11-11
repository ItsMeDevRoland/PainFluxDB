# PainFluxDB — Engine API

Everything the application team needs to build the Student Record Management
System on top of this engine.

PainFluxDB is a generic in-memory database engine. It stores
**databases → tables → columns → rows**. It knows nothing about students.
You define columns, you insert rows, you search/sort/queue them.

- Version: 0.1.0
- Standard: C++17
- Dependencies: none (standard library only)
- Thread safety: **not thread-safe** (see §9)

---

## 1. Build

Compile the engine together with your application:

```bash
g++ -std=c++17 -Wall -Wextra \
    DB.cpp table.cpp Row.cpp stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp facade.cpp \
    your_app.cpp -o student_system
```

If you only need part of the engine, the dependency graph is:

```
utility.cpp   → no deps
table.cpp     → utility
DB.cpp        → table, utility
Row.cpp       → table, stack, queue, utility
stack.cpp     → (none)
queue.cpp     → (none)
search.cpp    → table, utility
sort.cpp      → table, utility
stats.cpp     → table, utility
facade.cpp    → table, Row, search, utility
```

`main.cpp` is our dev harness. **Do not link it** — you have your own `main()`.

---

## 2. Headers

| Header | Gives you |
|---|---|
| `struct.hpp` | `Value`, `Column`, `Row`, `Node` (a table), `Database`, `ColumnType` |
| `config.hpp` | All limits (`TableColLimit`, `DatabaseLimit`, …) |
| `DB.hpp` | Create / use / delete / list databases |
| `table.hpp` | Create / search / delete tables, add columns |
| `Row.hpp` | Insert / find / update / delete rows, count rows |
| `stack.hpp` | Recently-deleted rows (LIFO) |
| `queue.hpp` | Pending rows (FIFO) |
| `search.hpp` | Linear and binary search |
| `sort.hpp` | Bubble / selection / insertion sort |
| `stats.hpp` | Min / max / sum / avg over a numeric column |
| `utility.hpp` | `makeInt`, `makeDouble`, `makeString`, `makeBool`, `valueToString`, `compareValue` |
| `facade.hpp` | **Ergonomic layer — see §19.** RowBuilder, cell access, typed find, one-call setup, printers |

You normally need: `DB.hpp`, `table.hpp`, `Row.hpp`, `stack.hpp`, `queue.hpp`,
`search.hpp`, `sort.hpp`, `stats.hpp`, `utility.hpp`, and `facade.hpp`.

---

## 3. Core Types

```cpp
enum ColumnType { COL_INT, COL_DOUBLE, COL_STRING, COL_BOOL };

struct Column {          // schema entry, positional
  string     ColName;
  ColumnType Type;
};

struct Value {           // one cell
  ColumnType Type;
  string     Str;
  double     Num;
  int        Int;
  bool       Bool;
};

struct Row {             // one record = one linked-list node
  Value Values[TableColLimit];  // positional, matches table->Columns[]
  int   ColumnCount;
  Row*  next;
};

struct Node {            // a TABLE (also a linked-list node)
  string  table;
  Column  Columns[TableColLimit];
  int     ColumnCount;
  Row*    RowHead;
  Row*    RowTail;
  int     RowCount;
  Database* DatabaseParent;
  Node*   next;
  StackNode* StackTop;   int StackCount;
  QueueNode* QueueFront; QueueNode* QueueRear; int QueueCount;
};
```

**Do not read or write `Values[]` by hand.** Use `FindColumn()` to get the
index, or the helpers in §4. Values are positional, so index 0 in a row is
whatever column 0 is in the table's schema.

---

## 4. Building Values

Never construct a `Value` by hand — the `Type` tag matters for comparison.

```cpp
Value makeInt(int v);
Value makeDouble(double v);
Value makeString(string v);
Value makeBool(bool v);

string valueToString(const Value& v);   // for printing
int    compareValue(const Value& a, const Value& b);  // -1 / 0 / 1
bool   valueEquals(const Value& a, const Value& b);
```

---

## 5. Databases

```cpp
bool       headlessCreateNewDatabase(string name);  // false if duplicate or full
bool       headlessUseDB(string name);              // selects it globally
bool       headlessDeleteDatabase(string name);     // frees its tables too
Database*  returnQueryADB(string name = "");        // nullptr if not found
string     returnNameQueryADB(string name = "");
Containers returnAllDB(string name = "");           // name="" → all
int        getDBSize();                             // count of used slots
int        findFreeDBSlot();                        // first empty index, -1 if full
```

Notes:
- `returnQueryADB` returns a **pointer into the global pool**, not a copy.
- `getDBSize()` is a **count**, not a valid write index. Slots can have holes
  after deletes. Use `findFreeDBSlot()` when you need somewhere to write.
- `returnAllDB("")` fills `ArrayContainer.DatabaseArray[0..DatabaseCount-1]`.

Example:

```cpp
headlessCreateNewDatabase("SCHOOL");
Database* db = returnQueryADB("SCHOOL");
if (db == nullptr) { /* creation failed */ }
```

---

## 6. Tables and Schema

```cpp
bool   CreateTable(Database* db, string tableName);
Node*  SearchTable(Database* db, string tableName);
bool   DeleteTable(Database* db, string tableName);
bool   DeleteTableRecursv(Database* db);
bool   DropDatabase(Database* db);
void   DisplayTables(Database* db, bool EnablePFdbDefaults = true);

bool   AddColumn(Node* table, string columnName, ColumnType type);
int    FindColumn(Node* table, string columnName);   // index or -1
```

Example — a student table:

```cpp
Database* db = returnQueryADB("SCHOOL");

CreateTable(db, "STUDENTS");
Node* students = SearchTable(db, "STUDENTS");

AddColumn(students, "ID",     COL_INT);
AddColumn(students, "FIRST",  COL_STRING);
AddColumn(students, "LAST",   COL_STRING);
AddColumn(students, "COURSE", COL_STRING);
AddColumn(students, "YEAR",   COL_INT);
AddColumn(students, "GPA",    COL_DOUBLE);
```

`CreateTable` refuses duplicate names. `AddColumn` refuses duplicates and
returns false once `TableColLimit` is reached.

---

## 7. Rows (CRUD)

```cpp
bool  InsertRow(Node* table, const Value* values, int count);
Row*  FindRow(Node* table, string columnName, const Value& match);
bool  UpdateRow(Node* table, Row* row, string columnName, const Value& newValue);
bool  DeleteRow(Node* table, string columnName, const Value& match);
int   CountRows(Row* head);                        // recursive
int   GetAllRows(Node* table, Row** out, int cap); // snapshot of pointers
```

### Insert

`values` must have exactly `table->ColumnCount` entries, **in schema order**.

```cpp
Value row[6];
row[0] = makeInt(2026005);
row[1] = makeString("Maria");
row[2] = makeString("Santos");
row[3] = makeString("BSCS");
row[4] = makeInt(2);
row[5] = makeDouble(1.75);

if (!InsertRow(students, row, 6)) { /* wrong shape or bad args */ }
```

### Find

```cpp
Row* r = FindRow(students, "ID", makeInt(2026005));
if (r != nullptr) {
    // read cells by index: use FindColumn once and cache it
    int gpaCol = FindColumn(students, "GPA");
    cout << valueToString(r->Values[gpaCol]) << endl;
}
```

### Update

```cpp
int gpaCol = FindColumn(students, "GPA");
UpdateRow(students, r, "GPA", makeDouble(1.80));
```

### Delete

```cpp
DeleteRow(students, "ID", makeInt(2026005));
```

**Deleting pushes the row onto the deleted-row stack automatically.** That is
how the "Recently Deleted" feature works — you do not call the stack yourself
on delete.

---

## 8. Stack — Recently Deleted (LIFO)

```cpp
bool PushDeleted(Node* table, Row* row);          // done for you by DeleteRow
Row* PopDeleted(Node* table);                     // you now own the row
Row* PeekDeleted(Node* table);                    // nullptr if empty
int  DisplayStack(Node* table, Row** out, int cap); // newest first
bool RestoreDeleted(Node* table);                 // undo delete
```

Example:

```cpp
Row* rows[MaxRowReturn];
int n = DisplayStack(students, rows, MaxRowReturn);

cout << "STACK:\nTop\n ↓\n";
for (int i = 0; i < n; i++) {
    cout << valueToString(rows[i]->Values[0]) << " "
         << valueToString(rows[i]->Values[2]) << endl;
}

// undo the most recent delete
RestoreDeleted(students);
```

**Ownership:** the stack **owns** the rows it holds. If you `PopDeleted()`, you
must `delete` the row yourself (or `RestoreDeleted()` to put it back).

---

## 9. Queue — Service Queue (FIFO)

```cpp
bool Enqueue(Node* table, Row* row);
Row* Dequeue(Node* table);                    // nullptr if empty
Row* PeekQueue(Node* table);                  // nullptr if empty
int  DisplayQueue(Node* table, Row** out, int cap);  // front first
bool RemoveFromQueue(Node* table, Row* row);
```

Example:

```cpp
Enqueue(students, FindRow(students, "ID", makeInt(2026001)));
Enqueue(students, FindRow(students, "ID", makeInt(2026005)));

Row* rows[MaxRowReturn];
int n = DisplayQueue(students, rows, MaxRowReturn);
for (int i = 0; i < n; i++) {
    cout << i + 1 << ". "
         << valueToString(rows[i]->Values[1]) << " "
         << valueToString(rows[i]->Values[2]) << endl;
}
```

**Ownership:** the queue only **borrows** rows. The table still owns them.
Deleting a queued row removes it from the queue automatically, so no dangling
pointers. Do **not** `delete` a row you got from `Dequeue()`.

---

## 10. Searching

```cpp
int LinearSearch(Node* table, string columnName, const Value& match,
                 Row** out, int cap);
int BinarySearch(Node* table, string columnName, const Value& match,
                 Row** out, int cap);
```

Both write up to `cap` matching `Row*` into `out` and return the **total**
number of matches (which may exceed `cap` — check it).

- **LinearSearch** — O(n), scans the chain once, works in any order.
- **BinarySearch** — builds a sorted snapshot then binary-searches it. The
  search itself is O(log n); the snapshot is O(n log n). Always correct, no
  need to sort first.

When to use which, for your documentation section:

| | Linear | Binary |
|---|---|---|
| Requires sorted data | No | Yes (handled internally) |
| Time complexity | O(n) | O(log n) search, O(n log n) prep |
| Works on linked list | Yes | Via snapshot |
| Best for | small / unsorted | large / repeated lookups |

Example:

```cpp
Row* found[MaxRowReturn];
int n = LinearSearch(students, "LAST", makeString("Santos"), found, MaxRowReturn);
for (int i = 0; i < n && i < MaxRowReturn; i++) {
    cout << valueToString(found[i]->Values[0]) << endl;
}
```

---

## 11. Sorting

```cpp
enum SortAlgorithm { BUBBLE, SELECTION, INSERTION };

bool SortRows(Node* table, string columnName, SortAlgorithm algo,
              bool ascending = true);
```

Sorts the table's row chain **in place by relinking**, so any `Row*` you were
holding stays valid (its position changes, its identity does not).

```cpp
SortRows(students, "ID",   BUBBLE,    true);   // by ID ascending
SortRows(students, "LAST", SELECTION, true);   // by last name A→Z
SortRows(students, "GPA",  INSERTION, false);  // by GPA descending
```

| Algorithm | Best | Average | Worst | Stable |
|---|---|---|---|---|
| Bubble | O(n) | O(n²) | O(n²) | Yes |
| Selection | O(n²) | O(n²) | O(n²) | No |
| Insertion | O(n) | O(n²) | O(n²) | Yes |

Sorting works on any column type. Strings compare lexicographically, numbers
numerically.

---

## 12. Statistics

```cpp
struct ColumnStats {
  double min, max, sum, avg;
  int    count;
};

ColumnStats ComputeStats(Node* table, string columnName);
```

```cpp
ColumnStats s = ComputeStats(students, "GPA");
cout << "Count: " << s.count << "\n";
cout << "Average GPA: " << s.avg << "\n";
cout << "Highest: " << s.max << "\n";
cout << "Lowest: " << s.min << "\n";
```

Only `COL_INT` and `COL_DOUBLE` columns are accepted; a text column returns
`count = 0`.

---

## 13. Limits

From `config.hpp`:

| Constant | Default | Meaning |
|---|---|---|
| `DatabaseLimit` | 100 | databases in the pool |
| `TableLimit` | 100 | tables per database |
| `TableColLimit` | 100 | columns per table |
| `MaxRowReturn` | 1024 | rows per search/sort snapshot |
| `MaxArrayContainer` | 100 | slots in `Containers` buckets |

A table holds as many rows as memory allows (linked list), but **search and
sort refuse to run above `MaxRowReturn`** because they use a fixed snapshot
buffer. Raise it in `config.hpp` if you need more.

---

## 14. Error Handling

Every function returns a value. **Check it.**

- `bool` functions return `false` on bad input (null table, unknown column,
  duplicate name, wrong row shape, full pool).
- Pointer functions return `nullptr` when nothing was found.
- Count functions return `0` when nothing matched.
- No function throws. No function exits the program.

---

## 15. Thread Safety and Reentrancy

**Not thread-safe.** The engine keeps global state (`databaseContainer`,
`ArrayContainer`) and `search.cpp` / `sort.cpp` use a `static` snapshot buffer.
Call from one thread only, and do not call `LinearSearch`/`BinarySearch`/
`SortRows` recursively from within a callback. Single-threaded console apps are
fine.

---

## 16. Complete Example

```cpp
#include "DB.hpp"
#include "Row.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "stats.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <iostream>
using namespace std;

int main() {
    headlessCreateNewDatabase("SCHOOL");
    Database* db = returnQueryADB("SCHOOL");

    CreateTable(db, "STUDENTS");
    Node* students = SearchTable(db, "STUDENTS");
    AddColumn(students, "ID",    COL_INT);
    AddColumn(students, "FIRST", COL_STRING);
    AddColumn(students, "LAST",  COL_STRING);
    AddColumn(students, "GPA",   COL_DOUBLE);

    struct Seed { int id; const char* first; const char* last; double gpa; };
    Seed seed[] = {
        {2026001, "Juan",  "Dela Cruz", 1.75},
        {2026003, "Pedro", "Cruz",      2.00},
        {2026005, "Maria", "Santos",    1.50},
    };

    for (auto& s : seed) {
        Value row[4] = { makeInt(s.id), makeString(s.first),
                         makeString(s.last), makeDouble(s.gpa) };
        InsertRow(students, row, 4);
    }

    cout << "All students (" << CountRows(students->RowHead) << "):\n";
    Row* all[MaxRowReturn];
    int n = GetAllRows(students, all, MaxRowReturn);
    for (int i = 0; i < n; i++) {
        cout << valueToString(all[i]->Values[0]) << "  "
             << valueToString(all[i]->Values[1]) << " "
             << valueToString(all[i]->Values[2]) << "  GPA "
             << valueToString(all[i]->Values[3]) << "\n";
    }

    SortRows(students, "LAST", SELECTION, true);
    cout << "\nSorted by last name:\n";
    n = GetAllRows(students, all, MaxRowReturn);
    for (int i = 0; i < n; i++)
        cout << valueToString(all[i]->Values[2]) << "\n";

    Row* found[MaxRowReturn];
    int hits = BinarySearch(students, "ID", makeInt(2026005), found, MaxRowReturn);
    cout << "\nBinary search 2026005: " << hits << " hit(s)\n";

    DeleteRow(students, "ID", makeInt(2026003));
    cout << "\nAfter delete, stack has " << students->StackCount << " row(s)\n";
    cout << "Top of stack: " << valueToString(PeekDeleted(students)->Values[2]) << "\n";

    Enqueue(students, FindRow(students, "ID", makeInt(2026001)));
    Enqueue(students, FindRow(students, "ID", makeInt(2026005)));
    cout << "\nService queue:\n";
    Row* q[MaxRowReturn];
    int qn = DisplayQueue(students, q, MaxRowReturn);
    for (int i = 0; i < qn; i++)
        cout << i + 1 << ". " << valueToString(q[i]->Values[2]) << "\n";

    ColumnStats st = ComputeStats(students, "GPA");
    cout << "\nGPA avg " << st.avg << "  min " << st.min << "  max " << st.max << "\n";

    headlessDeleteDatabase("SCHOOL");
    return 0;
}
```

Build and run:

```bash
g++ -std=c++17 -Wall -Wextra \
    DB.cpp table.cpp Row.cpp stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp \
    example.cpp -o example
./example
```

---

## 17. Ownership Cheat Sheet

| Where a `Row*` is | Who owns it | What you may do |
|---|---|---|
| In a table (`RowHead` chain) | the table | read, update; delete via `DeleteRow` |
| On the deleted stack | the stack | `PeekDeleted`, `RestoreDeleted`; if you `PopDeleted`, **you** must `delete` it |
| In the service queue | the table (queue borrows) | read only; **never** `delete` |
| Returned by `FindRow`/`GetAllRows`/searches | the table | read, update; never `delete` |

Freeing a table (`DeleteTable`, `DeleteTableRecursv`, `headlessDeleteDatabase`)
frees its rows, its stack nodes and its stack rows. Queue nodes are freed but
the rows are freed with the table's row chain.

---

## 18. Checklist Before You Integrate

- [ ] You call `headlessCreateNewDatabase` once at startup
- [ ] You fetch the `Database*` **after** creating it, never before
- [ ] You create your table(s) and columns once, not on every menu action
- [ ] Every `InsertRow` array has exactly `ColumnCount` entries in schema order
- [ ] You cache `FindColumn(...)` results instead of calling it in a loop
- [ ] You check every `bool` return
- [ ] You never `delete` a row you did not get from `PopDeleted`
- [ ] You do not link our `main.cpp`

---

## 19. The Facade — Write Less Code

`facade.hpp` is an optional convenience layer. It is purely additive: every
function wraps a core call, nothing in the core changes. Use it and skip the
positional `Value` arrays, the `makeInt()` noise, and the manual `FindColumn`
calls.

### 19.1 RowBuilder — build a row by column name

```cpp
#include "facade.hpp"

RowBuilder b(students);
b.setInt("ID", 2026005);
b.setString("FIRST", "Maria");
b.setString("LAST", "Santos");
b.setDouble("GPA", 1.75);

if (!b.commit()) {
    // a column is missing, or a value type does not match the schema
}
```

`setInt` / `setDouble` / `setString` / `setBool` return `false` when the column
does not exist **or** its declared type differs — a typo can never silently
write into the wrong cell.

`commit()` inserts a new row and fails unless **every** schema column was set.
`update(row)` applies only the columns you set and leaves the rest untouched:

```cpp
RowBuilder fix(students);
fix.setDouble("GPA", 1.80);
fix.update(FindRowInt(students, "ID", 2026005));
```

`filledCount()` reports how many columns have been set.

### 19.2 Named cell access

```cpp
string CellText   (Node* t, Row* r, string col);
int    CellInt    (Node* t, Row* r, string col);
double CellDouble (Node* t, Row* r, string col);
bool   CellBool   (Node* t, Row* r, string col);
bool   SetCell    (Node* t, Row* r, string col, const Value& v);
```

Unknown column or null row returns the type's default (`""`, `0`, `0.0`,
`false`) instead of crashing. `CellInt`/`CellDouble` convert between numeric
types, so a `COL_INT` read as a double still works.

```cpp
cout << CellText(students, row, "LAST") << " GPA " << CellDouble(students, row, "GPA");
```

### 19.3 Typed find / delete

```cpp
Row* FindRowInt   (Node* t, string col, int v);
Row* FindRowDouble(Node* t, string col, double v);
Row* FindRowStr   (Node* t, string col, string v);
Row* FindRowBool  (Node* t, string col, bool v);

bool DeleteRowInt   (Node* t, string col, int v);
bool DeleteRowDouble(Node* t, string col, double v);
bool DeleteRowStr   (Node* t, string col, string v);
bool DeleteRowBool  (Node* t, string col, bool v);
```

```cpp
if (DeleteRowInt(students, "ID", 2026005)) {
    // pushed onto the deleted stack automatically
}
```

### 19.4 One-call table setup

```cpp
TableSpec spec[] = {
    {"ID",    COL_INT},
    {"FIRST", COL_STRING},
    {"LAST",  COL_STRING},
    {"GPA",   COL_DOUBLE},
};

if (!SetupTable(db, "STUDENTS", spec, 4)) { /* bad db / conflicting column */ }
```

Creates the table if missing and adds any column not already present. Safe to
call on every startup: **idempotent**. A column that exists with a *different*
type is an error, so a contradictory schema is never silently accepted.

### 19.5 Schema-driven printing

```cpp
void DisplayAllRows(Node* table);                    // header + every row
void DisplayRows(Node* table, Row* const* rows, int count);  // a result set
void DisplayRow(Node* table, Row* row);              // one row
```

The header is generated from the schema, so it always matches the table:

```
ID | FIRST | LAST | GPA
1 | Juan | Dela Cruz | 1.75
2 | Maria | Santos | 1.5
```

All three accept null safely and print nothing.

### 19.6 Counters

```cpp
int  CountRowsOf(Node* table);   // recursive count, 0 for null
bool IsEmpty(Node* table);       // true for null or a table with no rows
```

### 19.7 Before and after

```cpp
// Core only
Value row[6];
row[0] = makeInt(id);
row[1] = makeString(first);
row[2] = makeString(last);
row[3] = makeString(course);
row[4] = makeInt(year);
row[5] = makeDouble(gpa);
InsertRow(students, row, 6);
cout << valueToString(FindRow(students, "ID", makeInt(id))->Values[5]) << endl;

// With the facade
RowBuilder b(students);
b.setInt("ID", id);
b.setString("FIRST", first);
b.setString("LAST", last);
b.setString("COURSE", course);
b.setInt("YEAR", year);
b.setDouble("GPA", gpa);
b.commit();
cout << CellText(students, FindRowInt(students, "ID", id), "GPA") << endl;
```

Both produce the same result. The facade version cannot get the column order
wrong.
