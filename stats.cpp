// Numeric column statistics.
// Deliberately generic: any COL_INT or COL_DOUBLE column can be summarised.

#include "stats.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"
#include <string>
using namespace std;

static double numericValue(const Value &v) {
  if (v.Type == COL_INT) {
    return static_cast<double>(v.Int);
  }
  return v.Num;
}

ColumnStats ComputeStats(Node *table, string columnName) {
  ColumnStats result;

  if (table == nullptr) {
    return result;
  }

  int columnIndex = FindColumn(table, columnName);
  if (columnIndex == -1) {
    return result;
  }

  // Refuse text columns; averaging strings is meaningless.
  ColumnType type = table->Columns[columnIndex].Type;
  if (type != COL_INT && type != COL_DOUBLE) {
    return result;
  }

  bool first = true;
  Row *current = table->RowHead;
  while (current != nullptr) {
    double value = numericValue(current->Values[columnIndex]);

    if (first) {
      result.min = value;
      result.max = value;
      first = false;
    } else {
      if (value < result.min) {
        result.min = value;
      }
      if (value > result.max) {
        result.max = value;
      }
    }

    result.sum += value;
    result.count += 1;
    current = current->next;
  }

  if (result.count > 0) {
    result.avg = result.sum / static_cast<double>(result.count);
  }

  return result;
}
