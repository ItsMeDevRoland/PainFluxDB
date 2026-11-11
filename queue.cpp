// Student Service Queue — generic FIFO row queue.
// Enqueue at the rear, dequeue from the front.
//
// The queue stores pointers, not copies: a queued row is still the same record
// the table holds. Ownership stays with the table.

#include "queue.hpp"
#include "struct.hpp"
using namespace std;

bool Enqueue(Node *table, Row *row) {
  if (table == nullptr || row == nullptr) {
    return false;
  }

  QueueNode *node = new QueueNode;
  node->row = row;
  node->next = nullptr;

  // Keep a rear pointer so enqueue is O(1) instead of walking the list.
  if (table->QueueRear == nullptr) {
    table->QueueFront = node;
    table->QueueRear = node;
  } else {
    table->QueueRear->next = node;
    table->QueueRear = node;
  }

  table->QueueCount += 1;
  return true;
}

Row *Dequeue(Node *table) {
  if (table == nullptr || table->QueueFront == nullptr) {
    return nullptr;
  }

  QueueNode *node = table->QueueFront;
  table->QueueFront = node->next;
  if (table->QueueFront == nullptr) {
    table->QueueRear = nullptr; // queue is empty again
  }
  table->QueueCount -= 1;

  // Only the node dies; the row still belongs to the table.
  Row *row = node->row;
  delete node;
  return row;
}

Row *PeekQueue(Node *table) {
  if (table == nullptr || table->QueueFront == nullptr) {
    return nullptr;
  }
  return table->QueueFront->row;
}

int DisplayQueue(Node *table, Row **out, int cap) {
  if (table == nullptr || out == nullptr || cap <= 0) {
    return 0;
  }

  int written = 0;
  QueueNode *current = table->QueueFront;
  while (current != nullptr && written < cap) {
    out[written] = current->row;
    written += 1;
    current = current->next;
  }
  return written;
}

bool RemoveFromQueue(Node *table, Row *row) {
  if (table == nullptr || row == nullptr) {
    return false;
  }

  QueueNode *previous = nullptr;
  QueueNode *current = table->QueueFront;

  while (current != nullptr) {
    if (current->row == row) {
      if (previous == nullptr) {
        table->QueueFront = current->next;
      } else {
        previous->next = current->next;
      }

      if (table->QueueRear == current) {
        table->QueueRear = previous;
      }

      table->QueueCount -= 1;
      delete current;
      return true;
    }

    previous = current;
    current = current->next;
  }

  return false;
}
