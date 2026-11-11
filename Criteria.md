# Criteria.md — Source of Truth

> Authoritative brief for the **PainFluxDB engine**.
> If code, docs, or a teammate disagree with this file, this file wins.
> Last reviewed: 2026-11-10 (v2 — engine-first framing corrected)

---

## 1. What PainFluxDB Is

**PainFluxDB is a generic, embeddable, in-memory database engine written in C++.**

It is **not** a student system. It has never heard of a "student", a "GPA", or a
"menu". It stores **databases → tables → columns → rows** and exposes operations
over them. That is the entire product.

The Student Record Management System is a **consumer** built by another team on
top of this engine. Our contract is a header and a set of functions. They call
in, they get data out.

Think of it this way:

| Layer | Owner | Analogy |
|---|---|---|
| **PainFluxDB engine** | **US** | We build SQLite. |
| Student Record app | Other team | They build the student app that links SQLite. |

Consequence: **the engine contains zero domain logic.** No `Student` struct, no
`addStudent()`, no student menu. If a feature only makes sense for students, it
belongs to the other team, not here.

---

## 2. Two-Team Split

| Layer | Owner | Responsibility |
|---|---|---|
| Engine | **US** | Storage, schema, rows, CRUD, search, sort, stack, queue, recursion, statistics |
| Application | Other team | Menus, prompts, input validation, formatting, student-specific logic |
| Graded system | Joint | The combined program submitted to the instructor |

The rubric marks the **combined system**. Our job is to supply the data
structures and algorithms they will call. If our engine is clean and documented,
their app is trivial to finish; if our engine is painful, their app is painful.

---

## 3. Rubric → Engine Capability

The course requires these concepts. As an engine we provide each one
generically, so the application team can demonstrate it through our API.

| Course requirement | Engine delivers | Where |
|---|---|---|
| Arrays | Fixed-capacity pools (`DatabaseLimit`, `TableColLimit`) | `config.hpp` |
| Structures / classes | `Database`, `Table`, `Row`, `Column`, `Value` | `struct.hpp` |
| Linked lists | Tables chained per database; rows chained per table | `table.cpp`, `row.cpp` |
| Stack | Recently-deleted rows (LIFO) | `stack.cpp` |
| Queue | Pending rows (FIFO) | `queue.cpp` |
| Linear search | Scan rows for a column match | `search.cpp` |
| Binary search | Search a sorted row snapshot | `search.cpp` |
| Sorting | Bubble / Selection / Insertion, by any column | `sort.cpp` |
| Recursion | Recursive row count, recursive table free | `recursion.cpp` |
| Pointers / dynamic memory | `new`/`delete` node lifecycle | all node code |
| Functions / modularity | One concern per translation unit | whole repo |
| OOP | Classes with encapsulated state | `struct.hpp` |

**Rubric weights:** Data Structures 25% + Searching/Sorting 15% = **40%**.
Protect these first. Formatting polish is the application team's problem.

---

## 4. Target Public API

This is the contract the application team codes against. Declarations live in
headers; every function is **headless** (no `cin`, no `cout`, no blocking).

### Database level (already implemented)
```cpp
bool        headlessCreateNewDatabase(string name);
bool        headlessUseDB(string name);
bool        headlessDeleteDatabase(string name);
Database*   returnQueryADB(string name = "");
string      returnNameQueryADB(string name = "");
Containers  returnAllDB(string name = "");
int         getDBSize();
int         findFreeDBSlot();
```

### Table level (already implemented, needs a header)
```cpp
bool    CreateTable(Database* db, string tableName);
Node*   SearchTable(Database* db, string tableName);
bool    DeleteTable(Database* db, string tableName);
bool    DeleteTableRecursv(Database* db);
bool    DropDatabase(Database* db);
void    DisplayTables(Database* db, bool EnablePFdbDefaults = true);
```

### Column / schema (to build)
```cpp
bool AddColumn(Table* t, string name, ColumnType type);
int  FindColumn(Table* t, string name);   // returns index or -1
```

### Row CRUD (to build)
```cpp
bool  InsertRow(Table* t, const Value* values);
Row*  FindRow(Table* t, string column, const Value& match);
bool  UpdateRow(Table* t, Row* row, string column, const Value& newValue);
bool  DeleteRow(Table* t, string column, const Value& match); // pushes to stack
int   CountRows(Row* head);                                   // recursive
```

### Search (to build)
```cpp
int   LinearSearch(Table* t, string column, const Value& match, Row** out, int cap);
int   BinarySearch(Table* t, string column, const Value& match, Row** out, int cap);
```

### Sort (to build)
```cpp
enum SortAlgorithm { BUBBLE, SELECTION, INSERTION };
void SortRows(Table* t, string column, SortAlgorithm algo, bool ascending);
```

### Stack — deleted rows (to build)
```cpp
bool  PushDeleted(Table* t, Row* row);
Row*  PopDeleted(Table* t);
Row*  PeekDeleted(Table* t);
int   DisplayStack(Table* t, Row** out, int cap);
```

### Queue — pending rows (to build)
```cpp
bool  Enqueue(Table* t, Row* row);
Row*  Dequeue(Table* t);
Row*  PeekQueue(Table* t);
int   DisplayQueue(Table* t, Row** out, int cap);
```

### Statistics (to build)
```cpp
struct ColumnStats { double min, max, sum, avg; int count; };
ColumnStats ComputeStats(Table* t, string numericColumn);
```

---

## 5. Current State Audit (2026-11-10)

| File | State |
|---|---|
| `config.hpp` | Works. Central limits panel. Added `MaxRowReturn`. |
| `struct.hpp` | **Rewritten.** `Value`/`Column`/`Row`/`StackNode`/`QueueNode`/`Node`(table)/`Database`. Row is now ~5.6 KB, table node ~4.1 KB. |
| `DB.cpp` / `DB.hpp` | Works. Ghost-table leak fixed — deletes now free tables. |
| `table.hpp` / `table.cpp` | Works. Table linked list + schema (`AddColumn`, `FindColumn`). Frees rows/stack/queue on delete. |
| `Row.hpp` / `Row.cpp` | **Rewritten.** Insert/Find/Update/Delete, recursive count, snapshot. Delete pushes to stack and pulls from queue. |
| `stack.hpp` / `stack.cpp` | **New.** LIFO deleted rows, plus `RestoreDeleted` undo. |
| `queue.hpp` / `queue.cpp` | **New.** FIFO pending rows, borrows rows, `RemoveFromQueue`. |
| `search.hpp` / `search.cpp` | **New.** Linear + binary search, any column, duplicate-aware. |
| `sort.hpp` / `sort.cpp` | **New.** Bubble/selection/insertion, any column, asc/desc, in-place relink. |
| `stats.hpp` / `stats.cpp` | **New.** Min/max/sum/avg/count on numeric columns. |
| `facade.hpp` / `facade.cpp` | **New.** Ergonomic layer: `RowBuilder`, named cell access, typed find/delete, `SetupTable`, schema printers, counters. |
| `facade_test.cpp` | **New.** 52 facade assertions, clean under ASan+UBSan. |
| `utility.hpp` / `utility.cpp` | Extended: value constructors, compare, format. |
| `engine_test.cpp` | **New.** 75 self-checking assertions, clean under ASan+UBSan. |
| `example_app.cpp` | **New.** Reference student app covering all 10 menu actions. |
| `ENGINE_API.md` | **New.** Full API contract for the application team. |
| `QUICKSTART.md` | **New.** 5-minute integration guide. |
| `main.cpp` | Dev harness, unchanged in behaviour. |
| `main.hpp` | Extern globals. |

Build: zero warnings with `-Wall -Wextra` across every file. Tests: 75 core +
52 facade. Memory: `sizeof(Row)` 5,616 B (was 2.98 MB), `sizeof(Node)` 4,120 B
(was 297 MB).

---

## 6. Limitations (documented, not hidden)

### 6.1 Memory model — RESOLVED
Was: `sizeof(Node)` = 297 MB because `RowContainer` was four `[100][100]`
arrays and `Node::Rows` was `RowContainer[100]`.
Now: one `Row` node per record (~5.6 KB, bounded by `TableColLimit` values),
schema stored once per table (~4.1 KB). A 100-row table costs ~560 KB instead
of ~29.7 GB.

### 6.2 Fixed caps, no growth
100 databases × 100 tables × 100 columns. Rows grow as a linked list, but
search/sort snapshots are capped at `MaxRowReturn` (1024). Stated design limit.

### 6.3 Ghost tables after delete — RESOLVED
`headlessDeleteDatabase()` now calls `DeleteTableRecursv()` before clearing the
name, so a reused slot starts empty. Covered by a test.

### 6.4 No persistence
In-memory only. Process exit loses everything.

### 6.5 Row layer — RESOLVED
Full CRUD exists and is tested. Rows are linked-list nodes owned by the table.

### 6.6 Not reentrant
`search.cpp`/`sort.cpp` share a `static` snapshot buffer. Single-threaded use
only. Documented in `ENGINE_API.md` §15.

### 6.7 Repo hygiene — PARTLY DONE
`main` binary untracked (`git rm --cached main`) and ignored. Repo still needs a
commit to be clean for submission.

---

## 7. Phases

**Phase 0 — Stabilize** ✅ done
- Fixed ghost-table leak; added headers for `table.cpp`.

**Phase 1 — Row model rewrite** ✅ done
- Replaced `RowContainer`/`Node::Rows` with `Table` + linked `Row` nodes.
- Schema: per-table column list (name + type).
- `InsertRow`, `FindRow`, `UpdateRow`, `DeleteRow`, `CountRows`, `GetAllRows`.

**Phase 2 — Algorithms** ✅ done
- Linear + binary search over rows by any column.
- Bubble / selection / insertion sort over rows by any column, asc/desc.

**Phase 3 — Stack + Queue** ✅ done
- Deleted-row stack (LIFO) + `RestoreDeleted` undo; pending-row queue (FIFO).
- Delete auto-pushes to stack and auto-removes from queue.

**Phase 4 — Statistics + recursion** ✅ done
- `ComputeStats` (min/max/sum/avg/count) on numeric columns.
- Recursive `CountRows`, recursive table free.

**Phase 5 — API documentation + handoff** ✅ done
- `ENGINE_API.md` (full contract incl. facade §19), `QUICKSTART.md`,
  `example_app.cpp`.
- Remaining: instructor-facing `Documentation.md` (sections I–XIII), flowchart,
  DS diagram, PDF export. That is the application team's deliverable to own,
  with our engine sections contributed.

---

## 8. Definition of Done

- [x] No `cin` / blocking reads in any function the application calls
- [x] Linked list used for tables and rows (real nodes, not arrays)
- [x] Stack and queue are real LIFO/FIFO structures
- [x] Linear and binary search both implemented and distinguishable
- [x] Three sorts, any column, both directions
- [x] At least one recursive function (`CountRows`, `DeleteTableRecursv`)
- [x] `sizeof(Row)` is small (~5.6 KB), not megabytes
- [x] Compiles `-Wall -Wextra` with zero warnings on our files
- [x] `ENGINE_API.md` complete enough that a stranger can use it
- [x] Ghost-table leak fixed
- [x] 75-assertion test suite, clean under ASan + UBSan
- [x] Facade layer (52 assertions) for front-end ergonomics
- [ ] `main` binary untracked, repo clean

**Engine is feature-complete as of 2026-11-10.** Outstanding: repo hygiene, and
the instructor-facing `Documentation.md` + flowchart + DS diagram + PDF, which
belongs to the combined deliverable.
