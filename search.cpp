// Row searching.
//
// LinearSearch  : O(n), works on any order, returns every match.
// BinarySearch  : O(log n) lookups, but the data must be ordered by the search
//                 column first. To keep the API safe for callers who forget to
//                 sort, this version builds its own sorted snapshot, which
//                 makes the whole call O(n log n) but always correct. The
//                 logarithmic part is the search itself.

#include "search.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <string>
using namespace std;

int LinearSearch(Node *table, string columnName, const Value &match, Row **out,
                 int cap) {
  if (table == nullptr || out == nullptr || cap <= 0) {
    return 0;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return 0;
  }

  int found = 0;
  Row *current = table->RowHead;
  while (current != nullptr) {
    if (valueEquals(current->Values[columnIndex], match)) {
      if (found < cap) {
        out[found] = current;
      }
      found += 1; // keep counting past the cap so the caller knows the true total
    }
    current = current->next;
  }
  return found;
}

// Insertion sort over a pointer array, used to build the ordered snapshot.
static void sortSnapshot(Row **rows, int count, int columnIndex) {
  for (int i = 1; i < count; i++) {
    Row *key = rows[i];
    int j = i - 1;
    while (j >= 0 &&
           compareValue(rows[j]->Values[columnIndex], key->Values[columnIndex]) >
               0) {
      rows[j + 1] = rows[j];
      j -= 1;
    }
    rows[j + 1] = key;
  }
}

int BinarySearch(Node *table, string columnName, const Value &match, Row **out,
                 int cap) {
  if (table == nullptr || out == nullptr || cap <= 0) {
    return 0;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return 0;
  }

  if (table->RowCount <= 0) {
    return 0;
  }

  // Snapshot into a fixed buffer so we never allocate per search.
  static Row *snapshot[MaxRowReturn];
  int count = 0;
  Row *current = table->RowHead;
  while (current != nullptr && count < MaxRowReturn) {
    snapshot[count] = current;
    count += 1;
    current = current->next;
  }

  sortSnapshot(snapshot, count, columnIndex);

  // Classic binary search for the first index whose value is not less than the
  // target, then walk forward to collect equal neighbours (duplicates).
  int low = 0;
  int high = count - 1;
  int firstMatch = -1;

  while (low <= high) {
    int mid = low + (high - low) / 2;
    int cmp = compareValue(snapshot[mid]->Values[columnIndex], match);

    if (cmp == 0) {
      firstMatch = mid;
      high = mid - 1; // keep searching left for the earliest duplicate
    } else if (cmp < 0) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  if (firstMatch == -1) {
    return 0;
  }

  int found = 0;
  int index = firstMatch;
  while (index < count &&
         valueEquals(snapshot[index]->Values[columnIndex], match)) {
    if (found < cap) {
      out[found] = snapshot[index];
    }
    found += 1;
    index += 1;
  }
  return found;
}
