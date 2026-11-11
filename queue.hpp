//
// queue.hpp — pending rows (FIFO)
// A service queue: enqueue at the rear, dequeue from the front.
//
// OWNERSHIP: the queue does NOT own its rows. It holds pointers to rows that
// still live in the table. The table frees them. If a queued row is deleted,
// DeleteRow removes it from the queue first so no dangling pointer is left.
//

#pragma once
#include "struct.hpp"
using namespace std;

bool Enqueue(Node *table, Row *row);
Row *Dequeue(Node *table);
Row *PeekQueue(Node *table);
int DisplayQueue(Node *table, Row **out, int cap);

// Removes `row` from the queue if present. Returns true if it was queued.
bool RemoveFromQueue(Node *table, Row *row);
