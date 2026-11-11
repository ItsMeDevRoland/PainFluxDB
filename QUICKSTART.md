# PainFluxDB — 5-Minute Quickstart

For the application team. Copy, paste, run.

---

## The whole engine in 30 lines

```cpp
#include "DB.hpp"
#include "Row.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <iostream>
using namespace std;

int main() {
    // 1. one database
    headlessCreateNewDatabase("SCHOOL");
    Database* db = returnQueryADB("SCHOOL");

    // 2. one table with a schema
    CreateTable(db, "STUDENTS");
    Node* t = SearchTable(db, "STUDENTS");
    AddColumn(t, "ID",   COL_INT);
    AddColumn(t, "NAME", COL_STRING);
    AddColumn(t, "GPA",  COL_DOUBLE);

    // 3. one row (values must be in schema order)
    Value row[3] = { makeInt(1), makeString("Maria"), makeDouble(1.75) };
    InsertRow(t, row, 3);

    // 4. read it back
    Row* r = FindRow(t, "ID", makeInt(1));
    cout << valueToString(r->Values[1]) << endl;   // Maria

    return 0;
}
```

Build:

```bash
g++ -std=c++17 DB.cpp table.cpp Row.cpp stack.cpp queue.cpp search.cpp sort.cpp stats.cpp utility.cpp facade.cpp app.cpp -o app
```

---

## Same thing with the facade (recommended)

```cpp
#include "DB.hpp"
#include "facade.hpp"
#include <iostream>
using namespace std;

int main() {
    headlessCreateNewDatabase("SCHOOL");
    Database* db = returnQueryADB("SCHOOL");

    TableSpec spec[] = {
        {"ID",   COL_INT},
        {"NAME", COL_STRING},
        {"GPA",  COL_DOUBLE},
    };
    SetupTable(db, "STUDENTS", spec, 3);
    Node* t = SearchTable(db, "STUDENTS");

    RowBuilder b(t);
    b.setInt("ID", 1);
    b.setString("NAME", "Maria");
    b.setDouble("GPA", 1.75);
    b.commit();

    cout << CellText(t, FindRowInt(t, "ID", 1), "NAME") << endl;  // Maria
    DisplayAllRows(t);

    return 0;
}
```

No positional arrays, no `makeInt` noise, no `FindColumn` calls. Details in
`ENGINE_API.md` §19.

---

## Six things that will bite you

1. **Fetch the `Database*` after creating it.** `returnQueryADB` before
   `headlessCreateNewDatabase` can hand back `nullptr` or a dead slot.
2. **Values are positional.** `row[0]` must match `Columns[0]`. Use
   `FindColumn(t, "GPA")` to get an index instead of hardcoding it — or use
   `RowBuilder` and skip this entirely.
3. **`InsertRow` needs the exact column count.** `ColumnCount` must equal
   `count`, or it returns `false`.
4. **`DeleteRow` already pushes to the stack.** Don't push manually.
5. **Queue borrows, stack owns.** Never `delete` a dequeued row.
6. **`getDBSize()` is a count, not an index.** Use `findFreeDBSlot()`.

---

## What maps to what menu item

| Menu item | Engine call | Facade shortcut |
|---|---|---|
| 1. Add Student | `InsertRow` | `RowBuilder::commit()` |
| 2. Display All | `GetAllRows` | `DisplayAllRows` |
| 3. Search Student | `LinearSearch` / `BinarySearch` | `FindRowInt` / `FindRowStr` |
| 4. Update Student | `FindRow` + `UpdateRow` | `RowBuilder::update()` |
| 5. Delete Student | `DeleteRow` | `DeleteRowInt` / `DeleteRowStr` |
| 6. Sort Students | `SortRows` | — |
| 7. Stack Operations | `DisplayStack` / `PopDeleted` / `RestoreDeleted` | — |
| 8. Queue Operations | `Enqueue` / `Dequeue` / `DisplayQueue` | — |
| 9. Student Statistics | `ComputeStats` | — |
| 10. Exit | `headlessDeleteDatabase` (optional cleanup) | — |

Full details: `ENGINE_API.md`.
