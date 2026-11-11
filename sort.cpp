// Row sorting.
//
// Each algorithm works on a pointer array snapshot, then rebuilds the linked
// list from the sorted order. Relinking keeps existing Row* handles valid,
// which matters because the application may hold references across a sort.
//
// Bubble    : O(n^2) comparisons, stable, easiest to explain.
// Selection : O(n^2) comparisons, O(n) swaps, not stable.
// Insertion : O(n^2) worst case, O(n) on nearly sorted data, stable.

#include "sort.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <string>
using namespace std;

static void bubbleSort(Row **rows, int count, int columnIndex, bool ascending) {
  for (int i = 0; i < count - 1; i++) {
    bool swapped = false;
    for (int j = 0; j < count - 1 - i; j++) {
      int cmp = compareValue(rows[j]->Values[columnIndex],
                             rows[j + 1]->Values[columnIndex]);
      bool outOfOrder = ascending ? (cmp > 0) : (cmp < 0);
      if (outOfOrder) {
        Row *temp = rows[j];
        rows[j] = rows[j + 1];
        rows[j + 1] = temp;
        swapped = true;
      }
    }
    if (!swapped) {
      break; // already sorted, stop early
    }
  }
}

static void selectionSort(Row **rows, int count, int columnIndex,
                          bool ascending) {
  for (int i = 0; i < count - 1; i++) {
    int target = i;
    for (int j = i + 1; j < count; j++) {
      int cmp = compareValue(rows[j]->Values[columnIndex],
                             rows[target]->Values[columnIndex]);
      bool better = ascending ? (cmp < 0) : (cmp > 0);
      if (better) {
        target = j;
      }
    }
    if (target != i) {
      Row *temp = rows[i];
      rows[i] = rows[target];
      rows[target] = temp;
    }
  }
}

static void insertionSort(Row **rows, int count, int columnIndex,
                          bool ascending) {
  for (int i = 1; i < count; i++) {
    Row *key = rows[i];
    int j = i - 1;
    while (j >= 0) {
      int cmp = compareValue(rows[j]->Values[columnIndex],
                             key->Values[columnIndex]);
      bool shift = ascending ? (cmp > 0) : (cmp < 0);
      if (!shift) {
        break;
      }
      rows[j + 1] = rows[j];
      j -= 1;
    }
    rows[j + 1] = key;
  }
}

bool SortRows(Node *table, string columnName, SortAlgorithm algo,
              bool ascending) {
  if (table == nullptr || table->RowCount < 2) {
    return table != nullptr;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return false;
  }

  if (table->RowCount > MaxRowReturn) {
    return false; // snapshot buffer would overflow
  }

  static Row *snapshot[MaxRowReturn];
  int count = 0;
  Row *current = table->RowHead;
  while (current != nullptr) {
    snapshot[count] = current;
    count += 1;
    current = current->next;
  }

  switch (algo) {
  case BUBBLE:
    bubbleSort(snapshot, count, columnIndex, ascending);
    break;
  case SELECTION:
    selectionSort(snapshot, count, columnIndex, ascending);
    break;
  case INSERTION:
  default:
    insertionSort(snapshot, count, columnIndex, ascending);
    break;
  }

  // Rebuild the chain in the new order and fix the tail.
  table->RowHead = snapshot[0];
  for (int i = 0; i < count - 1; i++) {
    snapshot[i]->next = snapshot[i + 1];
  }
  snapshot[count - 1]->next = nullptr;
  table->RowTail = snapshot[count - 1];

  return true;
}
