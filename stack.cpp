// Recently Deleted Students stack — generic deleted-row stack.
// Push on delete, pop to restore, peek to inspect the top.

#include "stack.hpp"
#include "struct.hpp"
using namespace std;

bool PushDeleted(Node *table, Row *row) {
  if (table == nullptr || row == nullptr) {
    return false;
  }

  StackNode *node = new StackNode;
  node->row = row;
  node->next = table->StackTop; // newest deletion becomes the new top
  table->StackTop = node;
  table->StackCount += 1;
  return true;
}

Row *PopDeleted(Node *table) {
  if (table == nullptr || table->StackTop == nullptr) {
    return nullptr;
  }

  StackNode *node = table->StackTop;
  table->StackTop = node->next;
  table->StackCount -= 1;

  Row *row = node->row;
  delete node; // the node dies, the row is handed back to the caller
  return row;
}

Row *PeekDeleted(Node *table) {
  if (table == nullptr || table->StackTop == nullptr) {
    return nullptr;
  }
  return table->StackTop->row;
}

bool RestoreDeleted(Node *table) {
  if (table == nullptr) {
    return false;
  }

  Row *row = PopDeleted(table);
  if (row == nullptr) {
    return false;
  }

  // Re-attach the row to the end of the table's row chain.
  row->next = nullptr;
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

// Writes top-first order so the caller can print a vertical stack with the
// newest entry at the top.
int DisplayStack(Node *table, Row **out, int cap) {
  if (table == nullptr || out == nullptr || cap <= 0) {
    return 0;
  }

  int written = 0;
  StackNode *current = table->StackTop;
  while (current != nullptr && written < cap) {
    out[written] = current->row;
    written += 1;
    current = current->next;
  }
  return written;
}
