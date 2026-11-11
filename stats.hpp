//
// stats.hpp — numeric column statistics
// Generic: works on any numeric column, not just GPA.
//

#pragma once
#include "struct.hpp"
#include <string>
using namespace std;

struct ColumnStats {
  double min = 0.0;
  double max = 0.0;
  double sum = 0.0;
  double avg = 0.0;
  int count = 0;
};

// Computes min/max/sum/avg over a numeric column. Non-numeric columns yield a
// zeroed result with count = 0.
ColumnStats ComputeStats(Node *table, string columnName);
