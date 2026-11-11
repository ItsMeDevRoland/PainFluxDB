//
// search.hpp — row searching
// LinearSearch scans the list once. BinarySearch works on a sorted snapshot,
// so it stays correct even if the table has not been sorted yet.
//

#pragma once
#include "struct.hpp"
#include <string>
using namespace std;

// Writes up to `cap` matching row pointers into `out`. Returns match count.
int LinearSearch(Node *table, string columnName, const Value &match, Row **out,
                 int cap);

// Same contract, but binary search over a sorted-by-column snapshot.
int BinarySearch(Node *table, string columnName, const Value &match, Row **out,
                 int cap);
