//
// row.hpp — row CRUD
// A row is a linked-list node holding positional Values[] that line up with the
// table's Columns[]. All functions are headless: arguments in, result out.
//

#pragma once
#include "struct.hpp"
#include <string>
using namespace std;

// Append a row. `values` must have `count` entries matching the schema order.
bool InsertRow(Node *table, const Value *values, int count);

// First row whose column `columnName` equals `match`, or nullptr.
Row *FindRow(Node *table, string columnName, const Value &match);

// Overwrite one cell of an existing row.
bool UpdateRow(Node *table, Row *row, string columnName, const Value &newValue);

// Remove the first matching row and push it onto the deleted-row stack.
bool DeleteRow(Node *table, string columnName, const Value &match);

// Recursive count of a row chain.
int CountRows(Row *head);

// Copy row pointers into `out` (up to `cap`). Returns how many were written.
int GetAllRows(Node *table, Row **out, int cap);
