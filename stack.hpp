//
// stack.hpp — recently deleted rows (LIFO)
// Deleting a row pushes it here. The stack owns the rows it holds, so a row is
// either in the table or in the stack, never both.
//

#pragma once
#include "struct.hpp"
using namespace std;

bool PushDeleted(Node *table, Row *row);
Row *PopDeleted(Node *table);
Row *PeekDeleted(Node *table);
int DisplayStack(Node *table, Row **out, int cap);

// Pops the newest deleted row and puts it back into the table. Returns false
// when the stack is empty. This is the "undo delete" operation.
bool RestoreDeleted(Node *table);
