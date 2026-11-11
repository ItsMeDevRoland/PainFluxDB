// This is the Row layer.
// Rows are linked-list nodes owned by their table. Deleting a row moves it to
// the table's deleted-row stack instead of freeing it, so the application can
// undo or inspect recent deletions.

#include "Row.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <string>
using namespace std;

bool InsertRow(Node *table, const Value *values, int count) {
  if (table == nullptr || values == nullptr) {
    return false;
  }

  if (count != table->ColumnCount) {
    return false; // row shape must match the schema
  }

  Row *row = new Row;
  row->ColumnCount = count;
  for (int i = 0; i < count; i++) {
    row->Values[i] = values[i];
  }
  row->next = nullptr;

  // Append at the tail so insertion is O(1) and order stays stable.
  if (table->RowHead == nullptr) {
    table->RowHead = row;
    table->RowTail = row;
  } else {
    table->RowTail->next = row;
    table->RowTail = row;
  }

  table->RowCount += 1;
  return true;
}

Row *FindRow(Node *table, string columnName, const Value &match) {
  if (table == nullptr) {
    return nullptr;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return nullptr;
  }

  Row *current = table->RowHead;
  while (current != nullptr) {
    if (valueEquals(current->Values[columnIndex], match)) {
      return current;
    }
    current = current->next;
  }
  return nullptr;
}

bool UpdateRow(Node *table, Row *row, string columnName, const Value &newValue) {
  if (table == nullptr || row == nullptr) {
    return false;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return false;
  }

  row->Values[columnIndex] = newValue;
  return true;
}

bool DeleteRow(Node *table, string columnName, const Value &match) {
  if (table == nullptr) {
    return false;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return false;
  }

  Row *previous = nullptr;
  Row *current = table->RowHead;

  while (current != nullptr) {
    if (valueEquals(current->Values[columnIndex], match)) {
      // Unlink first, then repair the tail if we removed the last node.
      if (previous == nullptr) {
        table->RowHead = current->next;
      } else {
        previous->next = current->next;
      }

      if (table->RowTail == current) {
        table->RowTail = previous;
      }

      current->next = nullptr;
      table->RowCount -= 1;

      // A deleted row must not stay queued, or the queue would hand out a
      // pointer to a row that now lives on the stack.
      RemoveFromQueue(table, current);

      // Deleting is the only path that fills the stack, so the stack is always
      // a truthful history of removals.
      PushDeleted(table, current);
      return true;
    }

    previous = current;
    current = current->next;
  }

  return false;
}

int CountRows(Row *head) {
  if (head == nullptr) {
    return 0;
  }

  return 1 + CountRows(head->next);
}

int GetAllRows(Node *table, Row **out, int cap) {
  if (table == nullptr || out == nullptr || cap <= 0) {
    return 0;
  }

  int written = 0;
  Row *current = table->RowHead;
  while (current != nullptr && written < cap) {
    out[written] = current;
    written += 1;
    current = current->next;
  }
  return written;
}
