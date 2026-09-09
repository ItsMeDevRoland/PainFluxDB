#include "utility.hpp"
#include "struct.hpp"
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
  for (int i = 0; i < Text.length(); i++) {
    Text[i] = std::toupper(Text[i]);
  }

  return Text;
}

void clearArrayContainer(){
    ArrayContainer = {};
}
