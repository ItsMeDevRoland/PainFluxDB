//
// sort.hpp — row sorting
// All three algorithms reorder the table's row chain in place by relinking
// nodes. Nothing is copied, so Row pointers handed out earlier stay valid.
//

#pragma once
#include "struct.hpp"
#include <string>
using namespace std;

enum SortAlgorithm { BUBBLE, SELECTION, INSERTION };

// Sorts `table`'s rows by `columnName`. Returns false on bad input.
bool SortRows(Node *table, string columnName, SortAlgorithm algo,
              bool ascending = true);
