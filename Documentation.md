# PainFluxDB — Documentation.md

**Function-by-function reference.** One section per file, one entry per
function: what it does, its parameters, what it returns, and when it fails.
Read this together with `ENGINE_API.md` (the published contract) — this file
explains *what each thing does*, that file explains *how to use the engine*.

- Engine: PainFluxDB v0.1.0 — C++17, standard library only
- Convention: `headless*` functions are the real work (no I/O); unprefixed
  interactive twins exist only for the dev harness
- Utility functions are `snake_case`; everything else is `PascalCase`

**Files covered**

1. `config.hpp` — limits (constants, no functions)
2. `struct.hpp` — data structures (no functions)
3. `utility.hpp` / `utility.cpp`
4. `DB.hpp` / `DB.cpp`
5. `table.hpp` / `table.cpp`
6. `Row.hpp` / `Row.cpp`
7. `stack.hpp` / `stack.cpp`
8. `queue.hpp` / `queue.cpp`
9. `search.hpp` / `search.cpp`
10. `sort.hpp` / `sort.cpp`
11. `stats.hpp` / `stats.cpp`
12. `facade.hpp` / `facade.cpp`
13. `main.hpp` / `main.cpp` — dev harness

---

## 1. `config.hpp` — The Control Panel

No functions. Every hard cap lives here:

| Constant | Value | Meaning |
|---|---|---|
| `TableColLimit` | 100 | columns per table; also the fixed width of every `Row::Values[]` |
| `DatabaseLimit` | 100 | databases the global pool can hold |
| `TableLimit` | 100 | tables per database (logical cap) |
| `EnableDevTool` | `true` | turns the dev CLI in `main.cpp` on/off |
| `MaxAliasSize` | 10 | aliases per CLI command (must not go below 5) |
| `MaxDBReturn` | 10 | databases one filtered query can return |
| `MaxTableSearch` | 100 | tables one search can return |
| `MaxArrayContainer` | 100 | slots in the shared `Containers` buckets |
| `MaxRowReturn` | 1024 | rows a search/sort snapshot buffer can hold |

Change these first when the application needs more headroom. The engine never
grows past them.

---

## 2. `struct.hpp` — The Data Structures

No functions. Seven structures build the whole engine:

- **`ColumnType`** — enum: `COL_INT`, `COL_DOUBLE`, `COL_STRING`, `COL_BOOL`.
- **`Value`** — one cell. A tagged struct: `Type` says which field is
  meaningful (`Str` / `Num` / `Int` / `Bool`). Never hand-build one; use the
  `make*` constructors in `utility.cpp`.
- **`Column`** — one schema entry: `ColName` + `Type`. Positional.
- **`Row`** — one record and one linked-list node: `Values[TableColLimit]`
  (index `i` matches schema column `i`), `ColumnCount`, `next`. About 5.6 KB.
- **`StackNode`** — one deleted-row stack entry: points at a `Row` it *owns*
  plus `next`.
- **`QueueNode`** — one pending-row queue entry: points at a `Row` it only
  *borrows* plus `next`.
- **`Node`** — a TABLE and a table-linked-list node: name, schema
  (`Columns[]` + `ColumnCount`), row chain (`RowHead`/`RowTail`/`RowCount`),
  parent database, `next` table, and its own stack (`StackTop`/`StackCount`)
  and queue (`QueueFront`/`QueueRear`/`QueueCount`). About 4.1 KB.
- **`Database`** — `DatabaseName` + `TableHead` of its table chain.
- **`Commands`** — dev-CLI command name + aliases.
- **`Containers`** — fixed-size return buckets (int/string/float arrays,
  `DatabaseArray`, `DatabaseCount`) so listing functions can hand results back
  without allocating.

---

## 3. `utility.hpp` / `utility.cpp` — Generic Helpers

Everything shared: value construction, comparison, formatting, container
housekeeping. All other modules include this.

### `int count_Array(Database *Array, int ArrayMaximumSize)`

Counts how many pool slots are in use. There is no `.size()` on a raw array,
so the engine fakes it.

- **Params:** `Array` — the database pool; `ArrayMaximumSize` — how far to scan.
- **Returns:** number of slots whose `DatabaseName` is not empty.
- **Used by:** `getDBSize()`.

### `string standardizedString(string Text)`

Returns an upper-cased copy of `Text` (per-character `toupper`).

- **Used by:** the dev CLI parser so commands are case-insensitive
  (`help` == `HELP`).

### `void clearArrayContainer()`

Resets the global `ArrayContainer` to an empty state (`DatabaseCount = 0`).

- **Why:** `returnAllDB()` clears before filling, so a filtered query can
  never leave stale pointers from a previous call.

### `int compareValue(const Value &a, const Value &b)`

Three-way comparison, the single comparison rule for the whole engine
(drives `FindRow`, both searches and all three sorts).

- **Returns:** `-1` if a < b, `0` if equal, `1` if a > b.
- **Rules:**
  - `COL_INT` vs `COL_INT` — numeric.
  - `COL_BOOL` vs `COL_BOOL` — `false` < `true`.
  - both non-string (any mix of INT/DOUBLE/BOOL) — compared as `double`, so
    mixed numeric columns still behave.
  - either side `COL_STRING` — lexicographic (dictionary) order.

### `bool valueEquals(const Value &a, const Value &b)`

Convenience wrapper: `compareValue(a, b) == 0`.

### `string valueToString(const Value &v)`

Renders one cell for printing.

- `COL_INT` — `to_string(Int)`.
- `COL_DOUBLE` — `to_string(Num)` with trailing zeros trimmed:
  `1.750000` becomes `1.75`, `2.000000` becomes `2`.
- `COL_BOOL` — `"true"` / `"false"`.
- `COL_STRING` — the string itself.

### `Value makeInt(int v)` / `makeDouble(double v)` / `makeString(string v)` / `makeBool(bool v)`

The **only** correct way to build a `Value` — they set the `Type` tag properly
so comparison and sorting work.

- `makeInt` fills both `Int` and `Num` (as double); `makeDouble` fills both
  `Num` and `Int` (truncated), so numeric comparisons never read garbage.

---

## 4. `DB.hpp` / `DB.cpp` — The Database Pool

Global state defined here:

- `Database databaseContainer[DatabaseLimit]` — the fixed pool (100 slots).
- `Database *DatabaseSelected` — the currently used database.
- `Containers ArrayContainer` — shared return buckets.

### `int getDBSize()`

Counts used pool slots via `count_Array`.

- **Returns:** how many databases exist.
- **Warning:** this is a **count**, not a valid index. Slots have holes after
  deletion — use `findFreeDBSlot()` to find somewhere to write.

### `int findFreeDBSlot()`

Scans slot 0 upward for the first slot with an empty name.

- **Returns:** the first free index, or `-1` when the pool is full.

### `bool headlessCreateNewDatabase(string name)`

The real database creation — no I/O.

- **Params:** `name` — the new database's name.
- **Returns:** `true` on success.
- **Fails (`false`) when:** the name already exists (duplicates would be
  unfindable) or no free slot remains.
- **Side effects:** writes the name into the free slot.

### `bool CreateNewDatabase(string MessageOverride = "")`

Interactive twin of `headlessCreateNewDatabase` (dev harness only). Prompts
for a name (or prints `MessageOverride` instead), guards against EOF/failed
`cin`, then delegates to the headless version and returns its result.

### `Database *returnQueryADB(string Name = "")`

Looks up one database by name.

- **Params:** `Name` — exact database name; `""` means slot 0.
- **Returns:** a **pointer into the global pool** (never a copy — a `Database`
  is 4 MB and copying would eat the stack), or `nullptr` if not found.
- **Note:** scans the **full** `DatabaseLimit` range, skipping holes left by
  deleted databases — scanning only `getDBSize()` slots would hide databases
  behind a hole.

### `string returnNameQueryADB(string Name = "")`

Name-only lookup: returns the database's `DatabaseName`, or `""` if the
database does not exist.

### `Containers returnAllDB(string Name = "")`

Lists databases into the shared return buckets.

- **Params:** `Name` — `""` lists **all**; otherwise only exact matches.
- **Returns:** a `Containers` whose `DatabaseArray[0..DatabaseCount-1]` holds
  the matches.
- **Behaviour:** clears `ArrayContainer` first (no stale pointers), scans the
  full range, skips holes.

### `void showAllDB()`

Dev display: prints a banner, then every existing database name using
`DatabaseCount` (not `getDBSize()`, because the container only holds what
exists).

### `bool headlessUseDB(string Name)`

Selects the globally "current" database — no I/O.

- **Params:** `Name` — database to select; an empty name deselects.
- **Returns:** `true` if a database was actually selected (or explicitly
  deselected with `""`), `false` if the name does not exist.
- **Side effects:** sets `DatabaseSelected` on match; sets it to `nullptr` on
  empty name or no match.

### `void UseDB(string InputOverride = "", string ErrorOverride = "", string SuccessOverride = "")`

Interactive twin of `headlessUseDB` (dev harness only). Prompts for a name
(showing the current selection), guards against EOF, rejects an empty query,
delegates to `headlessUseDB`, and prints success/error text — each message
replaceable via the override parameters (honoured when `EnableDevTool` is on).

### `bool headlessDeleteDatabase(string Name)`

Deletes a database and everything in it — no I/O.

- **Returns:** `true` if found and deleted, `false` if the name does not exist.
- **Order of operations (the ghost-table fix):**
  1. `DeleteTableRecursv(&slot)` — frees every table, row, stack and queue
     node **first**;
  2. clears the slot's name (a reused slot now starts empty);
  3. drops `DatabaseSelected` to `nullptr` if it pointed at the deleted
     database (no dangling "selected" pointer).

### `void DeleteDatabase(string InputOverride = "", string ErrorOverride = "", string SuccessOverride = "")`

Interactive twin of `headlessDeleteDatabase` (dev harness only). Prompts for
the name to delete, guards against EOF, delegates to the headless version,
prints success/error with override support.

---

## 5. `table.hpp` / `table.cpp` — Tables and Schema

A table is a linked-list node (`Node`) inside a database, and it owns its
rows, its deleted-row stack and its pending queue.

### `bool CreateTable(Database *db, string tableName)`

Creates a table and appends it to `db`'s table chain.

- **Returns:** `true` on success.
- **Fails (`false`) when:** `db` is `nullptr`, or a table with the same name
  already exists (duplicates would make `SearchTable` ambiguous).
- **Behaviour:** allocates a `Node`, sets its name and `DatabaseParent`, then
  appends it at the end of the chain (walks to the tail when the list is
  non-empty).

### `Node *SearchTable(Database *db, string tableName)`

Finds a table by exact name.

- **Returns:** the table's `Node*`, or `nullptr` if `db` is `nullptr` or the
  name does not exist.

### `bool DeleteTable(Database *db, string tableNameToDelete)`

Unlinks one table from the chain and frees **everything it owns**.

- **Returns:** `true` if found and deleted, `false` on bad input or not found.
- **Free order (the ownership rules in action):**
  1. every row in the row chain;
  2. every stack node **and the row inside it** (the stack owns those rows);
  3. every queue node **without** touching their rows (the queue only
     borrows; those rows were freed with the chain in step 1);
  4. the table node itself.

### `bool DeleteTableRecursv(Database *db)`

Empties a database's table chain: deletes the head table repeatedly until
none remain (`while (TableHead) DeleteTable(head)`).

- **Returns:** `true` (or `false` for `nullptr` input).
- **Used by:** `headlessDeleteDatabase` and `DropDatabase`.

### `bool DropDatabase(Database *db)`

Destroys a heap-allocated `Database` object: frees its tables first
(`DeleteTableRecursv`), then `delete db`.

- **Note:** pool databases are not `new`-allocated; this is for externally
  created `Database` objects. Returns `false` for `nullptr`.

### `void DisplayTables(Database *db, bool EnablePFdbDefaults = true)`

Dev display of a database's tables as `name (N rows)` lines.

- **Behaviour:** `nullptr` db — returns silently; empty chain — prints
  `ERROR INVALID OPERATIONS` only when defaults are enabled.

### `bool AddColumn(Node *table, string columnName, ColumnType type)`

Appends one schema entry at position `ColumnCount`.

- **Returns:** `true` on success.
- **Fails (`false`) when:** table is `nullptr`, the column limit
  (`TableColLimit`) is reached, or the column name already exists
  (duplicates would make `FindColumn` ambiguous).

### `int FindColumn(Node *table, string columnName)`

Resolves a column name to its positional index — every other module resolves
columns through this instead of hardcoding positions.

- **Returns:** the schema index of `columnName`, or `-1` when the table is
  `nullptr` or the column does not exist. `-1` is the universal
  "no such column" signal.

---

## 6. `Row.hpp` / `Row.cpp` — Row CRUD

Rows are linked-list nodes owned by their table. Deleting does not free — it
moves the row to the table's deleted-row stack.

### `bool InsertRow(Node *table, const Value *values, int count)`

Appends a new record at the **tail** of the row chain (O(1), insertion order
preserved).

- **Params:** `values` — array of cells; `count` — how many.
- **Returns:** `true` on success.
- **Fails (`false`) when:** table or values is `nullptr`, or `count` differs
  from `table->ColumnCount` (the row shape must match the schema exactly).
- **Behaviour:** copies the values positionally into a new `Row`, links it
  after `RowTail`, updates `RowHead`/`RowTail` and `RowCount`.

### `Row *FindRow(Node *table, string columnName, const Value &match)`

First row whose cell in `columnName` equals `match` (via `valueEquals`).

- **Returns:** the `Row*`, or `nullptr` when the table is `nullptr`, the
  column is unknown, or no row matches.
- **Ownership:** the returned row still belongs to the table — never `delete` it.

### `bool UpdateRow(Node *table, Row *row, string columnName, const Value &newValue)`

Overwrites one cell of an existing row.

- **Returns:** `true` on success; `false` when table/row is `nullptr` or the
  column is unknown. Complexity O(1) after the column lookup.

### `bool DeleteRow(Node *table, string columnName, const Value &match)`

Removes the **first** matching row — without freeing it.

- **Returns:** `true` if a row was removed, `false` on bad input, unknown
  column, or no match.
- **The delete choreography, in order:**
  1. unlink the node (repair `RowHead` or the previous node's `next`);
  2. repair `RowTail` if the deleted row was the last one;
  3. `RowCount -= 1`;
  4. `RemoveFromQueue(table, row)` — a deleted row must never stay queued,
     or the queue would hand out a pointer to a row that now lives on the
     stack;
  5. `PushDeleted(table, row)` — the row moves onto the deleted-row stack so
     the application can undo or inspect the deletion.

### `int CountRows(Row *head)`

**Recursive** row count — the rubric's recursion showcase.

```cpp
if (head == nullptr) return 0;
return 1 + CountRows(head->next);
```

- **Returns:** number of nodes in the chain; `0` for an empty/null chain.

### `int GetAllRows(Node *table, Row **out, int cap)`

Snapshots the row chain into a caller-provided pointer array (for printing,
queueing, etc.).

- **Params:** `out` — array to fill; `cap` — its size.
- **Returns:** how many pointers were written (stops at `cap` or chain end).
- **Fails:** returns `0` for `nullptr` inputs or `cap <= 0`.

---

## 7. `stack.hpp` / `stack.cpp` — Recently Deleted (LIFO)

The stack owns the rows it holds. A row is either in the table chain **or** on
the stack, never both. `DeleteRow` pushes automatically; the application never
has to.

### `bool PushDeleted(Node *table, Row *row)`

Pushes a row onto the table's deleted stack (newest becomes the new top).

- **Returns:** `true` on success; `false` when table or row is `nullptr`.
- **Side effects:** `StackTop`, `StackCount`.

### `Row *PopDeleted(Node *table)`

Removes the top entry **and transfers ownership of the row to the caller**.

- **Returns:** the row (caller must `delete` it), or `nullptr` when the table
  is `nullptr` or the stack is empty.
- **Side effects:** pops the `StackNode` (the node itself is deleted), updates
  `StackTop`/`StackCount`.

### `Row *PeekDeleted(Node *table)`

Looks at the top row without removing it.

- **Returns:** the top `Row*`, or `nullptr` when empty/null table.
- **Ownership:** still the stack's — read-only.

### `bool RestoreDeleted(Node *table)`

The **undo delete** operation: pops the newest deleted row and re-appends it
to the end of the table's row chain (head/tail repaired, `RowCount += 1`).

- **Returns:** `true` when a row was restored; `false` when the table is
  `nullptr` or the stack was empty.

### `int DisplayStack(Node *table, Row **out, int cap)`

Snapshots the stack into `out`, **newest first** (so the caller can print a
vertical stack with the newest entry on top).

- **Returns:** how many pointers were written, up to `cap`.

---

## 8. `queue.hpp` / `queue.cpp` — Service Queue (FIFO)

A waiting line of rows. **The queue only borrows rows** — they still belong to
the table chain, so dequeuing frees only the queue node, never the row.

### `bool Enqueue(Node *table, Row *row)`

Appends a row at the **rear** of the queue — O(1) because the table caches
`QueueRear` (no walking).

- **Returns:** `true` on success; `false` when table or row is `nullptr`.
- **Side effects:** `QueueFront`/`QueueRear`/`QueueCount`.

### `Row *Dequeue(Node *table)`

Removes and returns the row at the **front** of the queue.

- **Returns:** the front `Row*`, or `nullptr` when the table is `nullptr` or
  the queue is empty.
- **Ownership:** the row is still owned by the **table** — only the `QueueNode`
  is deleted. Never `delete` a dequeued row.

### `Row *PeekQueue(Node *table)`

Looks at the front row without removing it. `nullptr` when empty/null table.

### `int DisplayQueue(Node *table, Row **out, int cap)`

Snapshots the queue into `out`, **front first** (serving order).

- **Returns:** how many pointers were written, up to `cap`.

### `bool RemoveFromQueue(Node *table, Row *row)`

Unlinks one specific row from anywhere in the queue, by pointer identity.

- **Returns:** `true` if the row was queued (and is now removed), `false`
  otherwise.
- **Repairs:** `QueueFront` when removing the head, `QueueRear` when removing
  the tail; deletes only the queue node.
- **Why it exists:** called automatically by `DeleteRow` so a deleted row can
  never be handed out by the queue later (no dangling pointers — tested).

---

## 9. `search.hpp` / `search.cpp` — Searching

Both functions share one contract: write up to `cap` matching `Row*` into
`out` and return the **total** match count (which may exceed `cap` — extra
matches are counted but not written, so the caller always knows the true
total).

### `static void sortSnapshot(Row **rows, int count, int columnIndex)` *(internal)*

Insertion sort over a pointer array by one column, used by `BinarySearch` to
order the snapshot.

### `int LinearSearch(Node *table, string columnName, const Value &match, Row **out, int cap)`

Scans the row chain once, collecting every match. Works on data in any order.

- **Complexity:** O(n).
- **Returns:** total number of matches (0 when table is `nullptr`, output
  buffer invalid, `cap <= 0`, or the column is unknown).

### `int BinarySearch(Node *table, string columnName, const Value &match, Row **out, int cap)`

Binary search over a sorted snapshot — always correct even if the table was
never sorted, because it sorts its own snapshot first.

- **Steps:**
  1. copy row pointers into a fixed `static` buffer (up to `MaxRowReturn`);
  2. insertion-sort the snapshot by the chosen column;
  3. classic binary search for the **first** index whose value is not less
     than the target (keeps searching left after a hit — earliest duplicate);
  4. walk right from that index collecting every equal value.
- **Complexity:** O(n log n) total (snapshot + sort); the search itself is
  O(log n).
- **Returns:** total matches; `0` on bad input, unknown column, or empty table.
- **Landmine:** the snapshot buffer is a function-local `static` array —
  searching is **not reentrant**; the engine is single-threaded only.

---

## 10. `sort.hpp` / `sort.cpp` — Sorting

```cpp
enum SortAlgorithm { BUBBLE, SELECTION, INSERTION };
```

All three algorithms sort a **pointer snapshot** and then rebuild the row
chain by relinking — the row objects never move in memory, so any `Row*` the
application held before the sort stays valid.

### `static void bubbleSort(...)` *(internal)*

Adjacent-pair swaps in the chosen direction; stops early when a full pass
makes no swap (already sorted → O(n) best case). Stable.

### `static void selectionSort(...)` *(internal)*

Finds the best remaining element (minimum for ascending, maximum for
descending) in the unsorted suffix and swaps it into place — one swap per
pass, O(n) swaps total. Not stable.

### `static void insertionSort(...)` *(internal)*

Shifts out-of-order elements right and drops the key into the gap; O(n) on
nearly-sorted data. Stable.

### `bool SortRows(Node *table, string columnName, SortAlgorithm algo, bool ascending = true)`

Sorts the table's rows in place by any column, ascending or descending.

- **Returns:** `true` on success.
- **Fails (`false`) when:** table is `nullptr`, the column is unknown, or
  `RowCount > MaxRowReturn` (the snapshot buffer would overflow — refuses
  rather than corrupt).
- **Note:** a table with fewer than 2 rows returns `true` immediately (nothing
  to do).
- **Behaviour:** snapshot pointers → run the chosen algorithm (all delegate
  comparisons to `compareValue`, so numbers compare numerically and strings
  lexicographically) → rebuild the chain (`RowHead`, relinked `next` pointers,
  `RowTail` fixed to the last element).

---

## 11. `stats.hpp` / `stats.cpp` — Statistics

### `struct ColumnStats`

`min`, `max`, `sum`, `avg` (doubles) + `count` (int). A zeroed struct with
`count = 0` is the "nothing computed" signal.

### `static double numericValue(const Value &v)` *(internal)*

Reads a cell as a double: `COL_INT` casts `Int`; otherwise `Num`.

### `ColumnStats ComputeStats(Node *table, string columnName)`

One pass over the rows: initialises `min`/`max` from the first row, updates
them per row, accumulates `sum` and `count`, then `avg = sum / count`.

- **Returns:** the stats; a **zeroed result (`count = 0`)** when the table is
  `nullptr`, the column is unknown, or the column type is not `COL_INT` /
  `COL_DOUBLE` (averaging text is meaningless — the engine refuses instead of
  guessing).

---

## 12. `facade.hpp` / `facade.cpp` — The Ergonomic Layer

A purely additive wrapper over the core: every function delegates to a core
call, nothing in the core changed. It exists so the application team can skip
positional `Value` arrays, `make*` noise and manual `FindColumn` calls.

### Group 1 — `RowBuilder` (class)

Fill a row **by column name** — order never matters.

#### `bool RowBuilder::setInt(string col, int v)` / `setDouble` / `setString` / `setBool`

Records one cell value for a column.

- **Returns:** `false` when the table is `nullptr`, the column does not exist,
  **or** the value's kind does not match the column's declared schema type —
  a typo can never silently write into the wrong cell.
- **Side effects:** stores the properly-tagged `Value`; `filledCount()`
  increases when the column is set for the first time.

#### `bool RowBuilder::commit()`

Inserts a brand-new row.

- **Returns:** `false` unless **every** schema column was set (a partial row
  would be rejected by `InsertRow` anyway — this catches the mistake at the
  call site); also `false` for a `nullptr` table or a failed `InsertRow`.

#### `bool RowBuilder::update(Row *row)`

Applies **only the columns set so far** onto an existing row (untouched
columns stay as they are).

- **Returns:** `true` when at least one column was applied; `false` for
  `nullptr` inputs or nothing set.

#### `int RowBuilder::filledCount() const`

How many columns have been marked as set (diagnostics).

### Group 2 — Named cell access

#### `static const Value *cellAt(Node *table, Row *row, string col)` *(internal)*

Resolves a column to a cell pointer; `nullptr` for `nullptr` inputs, unknown
column, or an index beyond the row's width.

#### `string CellText(Node *table, Row *row, string col)`

Reads a cell as text. `""` for unknown column / null row; `COL_INT` renders
via `to_string`, everything else via `valueToString`.

#### `int CellInt(Node *table, Row *row, string col)`

Reads a cell as int. `0` on unknown/null; converts `COL_DOUBLE` by truncation.

#### `double CellDouble(Node *table, Row *row, string col)`

Reads a cell as double. `0.0` on unknown/null; converts `COL_INT` by widening.

#### `bool CellBool(Node *table, Row *row, string col)`

Reads a cell as bool. `false` on unknown/null or a non-bool column.

#### `bool SetCell(Node *table, Row *row, string col, const Value &v)`

Named write: `true` when the column exists and the write happened (delegates
to `UpdateRow`).

### Group 3 — Typed find / delete

#### `Row *FindRowInt(Node *table, string col, int v)` / `FindRowDouble` / `FindRowStr` / `FindRowBool`

`FindRow` without the `make*` boilerplate — builds the correctly-tagged
`Value` for you. Same ownership rules: the row still belongs to the table.

#### `bool DeleteRowInt(...)` / `DeleteRowDouble` / `DeleteRowStr` / `DeleteRowBool`

`DeleteRow` with typed values; deleting still auto-removes from the queue and
auto-pushes to the stack.

### Group 4 — One-call table setup

#### `struct TableSpec`

`{ string name; ColumnType type; }` — one desired schema column.

#### `bool SetupTable(Database *db, string tableName, const TableSpec *spec, int specCount)`

Creates the table if missing and adds every column in `spec` that is not
already present — **idempotent**, safe to call on every startup.

- **Returns:** `true` when the schema is satisfied.
- **Fails (`false`) when:** `db`/`spec` is `nullptr`, `specCount < 0`, table
  creation fails, or an existing column has a **different type** than the spec
  (a contradictory schema is never silently accepted).

### Group 5 — Schema-driven printing

#### `void DisplayAllRows(Node *table)`

Prints a header generated from the schema, then every row
(`valueToString` cells joined with `" | "`). Null table prints nothing.

#### `void DisplayRows(Node *table, Row *const *rows, int count)`

Same output for a result set (a search result, for example).

#### `void DisplayRow(Node *table, Row *row)`

Header + one row, for one-off prints. All three accept null safely.

### Group 6 — Counters

#### `int CountRowsOf(Node *table)`

Recursive row count without touching `RowHead` yourself; `0` for null.

#### `bool IsEmpty(Node *table)`

`true` for a `nullptr` table **or** a table with no rows.

---

## 13. `main.hpp` / `main.cpp` — Dev Harness (do not link into your app)

`main.hpp` declares the engine globals other translation units may use:
`databaseContainer[DatabaseLimit]`, `ArrayContainer`, `DatabaseSelected`,
and `showAllDB()`.

`main.cpp` is a development CLI, not part of the engine library.

### `string showHelp()`

Returns the help text. With `EnableDevTool == false` a minimal banner; with it
enabled, the full CLI command list (`CREATEDB`, `EXIT`, `HELP`, `SHOWALLDB`,
`USE`, `DELETEDB`).

### `void HandleParser(string Input)`

Dispatches one standardized (upper-cased) command to its interactive function:
`CREATEDB` → `CreateNewDatabase()`, `SHOWALLDB` → `showAllDB()`, `HELP` →
prints `showHelp()`, `USE` → `UseDB()`, `DELETEDB` → `DeleteDatabase()`.
Unknown commands are ignored silently.

### `bool DevTool()`

The REPL loop: prints a banner, reads a line (guarded against EOF — a failed
`cin` closes the CLI instead of spinning forever), upper-cases it, exits on
`EXIT` (`"Bye bye!"`), otherwise dispatches via `HandleParser`.

### `int main()`

Runs `DevTool()` when `EnableDevTool` is on, then returns 0.

---

## 14. Cross-Cutting Rules (apply to every function above)

1. **Null-safety everywhere.** Every public function validates its pointer
   arguments and returns `false` / `nullptr` / `0` instead of crashing.
2. **No function throws or exits.** Every failure is a return value — check it.
3. **Headless discipline.** Engine functions never read `cin`; `cout` appears
   only in `Display*`/interactive twins meant for the dev harness.
4. **Positional values.** Cell `i` of a row belongs to schema column `i`;
   resolve names through `FindColumn` (or the facade) and cache the index.
5. **Ownership is fixed.** Table owns its rows; stack owns deleted rows
   (until popped or restored); queue borrows. Callers only ever `delete` a
   row they got from `PopDeleted`.
6. **Single-threaded.** Global state plus the `static` search/sort snapshots
   make the engine non-reentrant — one thread, no nested search/sort calls.
