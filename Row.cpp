// This is the Row Modification Cpp!
// Only used to modify the row within a table, it has no purpose other than that
// purpose
//

#include "struct.hpp"
#include <string>
using namespace std;

// Find the first Match within the database, Hence "FROW", USE 3 LETTER WORD FOR
// DATATYPE, EX: INT, STR, DOB, BOL
RowContainer *findFRowMatch(Database *db, Node *table, string ItemLooking = "",
                            string dataType = "") {
  if (db != nullptr && table != nullptr) {
    for (RowContainer row : table->Rows) {
      if (dataType == "INT") {
        for (auto &colI : table->Rows->ColInt) {
          if (colI->ColName == ItemLooking) {
            return &row;
          }
        }
      } else if (dataType == "STR") {
        for (auto &colS : table->Rows->ColString) {
          if (colS->ColName == ItemLooking) {
            return &row;
          }
        }
      } else if (dataType == "DOB") {
        for (auto &colD : table->Rows->ColDouble) {
          if (colD->ColName == ItemLooking) {
            return &row;
          }
        }
      } else if (dataType == "BOL") {
        for (auto &colB : table->Rows->ColBool) {
          if (colB->ColName == ItemLooking) {
            return &row;
          }
        }
      } else {
        return nullptr;
      }
    }
  }
  return nullptr;
}

// Push a data on the top, ## STRING VARIANT ##
bool PushRowDataSTR(RowContainer parentRow, string ColName) {}