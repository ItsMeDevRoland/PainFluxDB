//
// table.hpp — table level API
// A table is a linked-list node inside a database. It owns its schema, its
// rows, its deleted-row stack and its pending queue.
//

#pragma once
#include "struct.hpp"
#include <string>
using namespace std;

bool CreateTable(Database *db, string tableName);
Node *SearchTable(Database *db, string tableName);
bool DeleteTable(Database *db, string tableName);
bool DeleteTableRecursv(Database *db);
bool DropDatabase(Database *db);
void DisplayTables(Database *db, bool EnablePFdbDefaults = true);

// Schema. Columns are positional; a row's Values[] lines up with Columns[].
bool AddColumn(Node *table, string columnName, ColumnType type);
int FindColumn(Node *table, string columnName);
