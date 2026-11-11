#include "utility.hpp"
#include "struct.hpp"
#include <cctype>
#include <string>

using namespace std;

// since theres no .size() on raw array, we'll fake it
int count_Array(Database *Array, int ArrayMaximumSize) {
  int count = 0;
  for (int item = 0; item < ArrayMaximumSize; item++) {
    if (!Array[item].DatabaseName.empty()) {
      count += 1;
    }
  };
  return count;
}

// Standardized the string to ensure all is upper text, used for parsing and
// lexer
string standardizedString(string Text) {
  for (size_t i = 0; i < Text.length(); i++) {
    Text[i] = std::toupper(static_cast<unsigned char>(Text[i]));
  }

  return Text;
}

void clearArrayContainer() {
  ArrayContainer = {};
  ArrayContainer.DatabaseCount = 0;
}

// Numeric columns compare as numbers, string columns compare lexicographically.
// The return value follows the usual -1 / 0 / 1 convention so every sorting
// algorithm can share it.
int compareValue(const Value &a, const Value &b) {
  if (a.Type == COL_INT && b.Type == COL_INT) {
    if (a.Int < b.Int)
      return -1;
    if (a.Int > b.Int)
      return 1;
    return 0;
  }

  if (a.Type == COL_BOOL && b.Type == COL_BOOL) {
    if (a.Bool == b.Bool)
      return 0;
    return a.Bool ? 1 : -1;
  }

  // Anything else is compared as a double so mixed INT/DOUBLE columns still
  // behave sensibly, with strings falling back to text order below.
  if (a.Type != COL_STRING && b.Type != COL_STRING) {
    double left = (a.Type == COL_INT) ? static_cast<double>(a.Int) : a.Num;
    double right = (b.Type == COL_INT) ? static_cast<double>(b.Int) : b.Num;
    if (left < right)
      return -1;
    if (left > right)
      return 1;
    return 0;
  }

  if (a.Str < b.Str)
    return -1;
  if (a.Str > b.Str)
    return 1;
  return 0;
}

bool valueEquals(const Value &a, const Value &b) {
  return compareValue(a, b) == 0;
}

string valueToString(const Value &v) {
  switch (v.Type) {
  case COL_INT:
    return to_string(v.Int);
  case COL_DOUBLE: {
    string out = to_string(v.Num);
    // to_string always emits six decimals; trim them so output stays readable.
    size_t dot = out.find('.');
    if (dot != string::npos) {
      size_t last = out.find_last_not_of('0');
      if (last == string::npos || last <= dot) {
        out.erase(dot); // whole fractional part was zeros: "2.000000" -> "2"
      } else {
        out.erase(last + 1); // "1.750000" -> "1.75"
      }
    }
    return out;
  }
  case COL_BOOL:
    return v.Bool ? "true" : "false";
  case COL_STRING:
  default:
    return v.Str;
  }
}

Value makeInt(int v) {
  Value out;
  out.Type = COL_INT;
  out.Int = v;
  out.Num = static_cast<double>(v);
  return out;
}

Value makeDouble(double v) {
  Value out;
  out.Type = COL_DOUBLE;
  out.Num = v;
  out.Int = static_cast<int>(v);
  return out;
}

Value makeString(string v) {
  Value out;
  out.Type = COL_STRING;
  out.Str = v;
  return out;
}

Value makeBool(bool v) {
  Value out;
  out.Type = COL_BOOL;
  out.Bool = v;
  return out;
}
