# PainFluxDB

A small, embeddable, in-memory database engine written in C++17.
Stores **databases → tables → columns → rows** with a linked-list row store,
a deleted-row stack, a pending-row queue, searching, sorting and statistics.

No dependencies. No build system needed — one `g++` line.

```bash
g++ -std=c++17 -Wall -Wextra \
    DB.cpp table.cpp Row.cpp stack.cpp queue.cpp search.cpp sort.cpp \
    stats.cpp utility.cpp facade.cpp your_app.cpp -o your_app
```

## Where to start

| You want to… | Read |
|---|---|
| Integrate the engine in 5 minutes | [`QUICKSTART.md`](QUICKSTART.md) |
| Know every function, type and rule | [`ENGINE_API.md`](ENGINE_API.md) |
| See a complete working program | [`example_app.cpp`](example_app.cpp) |
| Understand scope and design decisions | [`Criteria.md`](Criteria.md) |
| Verify it works | [`engine_test.cpp`](engine_test.cpp) |

## Verify

```bash
# core engine — 75 assertions
g++ -std=c++17 -Wall -Wextra engine_test.cpp DB.cpp table.cpp Row.cpp \
    stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp -o test
./test

# facade layer — 52 assertions
g++ -std=c++17 -Wall -Wextra facade_test.cpp facade.cpp DB.cpp table.cpp \
    Row.cpp stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp -o ftest
./ftest
```

Expected: `PASSED: 75   FAILED: 0` and `PASSED: 52   FAILED: 0`.

## What the engine provides

| Area | Functions |
|---|---|
| Databases | `headlessCreateNewDatabase`, `headlessUseDB`, `headlessDeleteDatabase`, `returnQueryADB`, `returnAllDB` |
| Tables & schema | `CreateTable`, `SearchTable`, `DeleteTable`, `AddColumn`, `FindColumn`, `SetupTable` |
| Rows | `InsertRow`, `FindRow`, `UpdateRow`, `DeleteRow`, `CountRows`, `GetAllRows` |
| **Facade** | `RowBuilder`, `CellText`/`CellInt`/`CellDouble`, `FindRowInt`/`FindRowStr`, `DisplayAllRows`, `CountRowsOf` |
| Stack (LIFO) | `PushDeleted`, `PopDeleted`, `PeekDeleted`, `DisplayStack`, `RestoreDeleted` |
| Queue (FIFO) | `Enqueue`, `Dequeue`, `PeekQueue`, `DisplayQueue`, `RemoveFromQueue` |
| Search | `LinearSearch`, `BinarySearch` |
| Sort | `SortRows` (bubble / selection / insertion, any column, asc/desc) |
| Statistics | `ComputeStats` (min / max / sum / avg / count) |

## Data structures

- **Linked list** — tables chained per database, rows chained per table
- **Stack** — recently deleted rows, owned by the stack until restored
- **Queue** — pending rows, borrows rows from the table
- **Arrays** — fixed-capacity database pool and per-table schema
- **Recursion** — `CountRows`, `DeleteTableRecursv`

## Limits

Configured in [`config.hpp`](config.hpp): 100 databases, 100 tables per
database, 100 columns per table, 1024 rows per search/sort snapshot.
Rows themselves grow unbounded as a linked list.

## License

Apache 2.0.
