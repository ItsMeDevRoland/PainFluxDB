// example_app.cpp — the reference integration.
// This is what the application team should copy as a starting point.
// It implements every menu action headlessly against the engine, so it doubles
// as proof that the engine covers the whole project brief.

#include "DB.hpp"
#include "Row.hpp"
#include "config.hpp"
#include "queue.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "stack.hpp"
#include "stats.hpp"
#include "struct.hpp"
#include "table.hpp"
#include "utility.hpp"

#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

static Database *DB = nullptr;
static Node *Students = nullptr;

// Schema indices, resolved once at startup instead of on every access.
static int COL_ID = -1;
static int COL_FIRST = -1;
static int COL_LAST = -1;
static int COL_COURSE = -1;
static int COL_YEAR = -1;
static int COL_GPA = -1;

static void printRow(Row *row) {
  cout << left << setw(10) << valueToString(row->Values[COL_ID]) << setw(12)
       << valueToString(row->Values[COL_FIRST]) << setw(12)
       << valueToString(row->Values[COL_LAST]) << setw(10)
       << valueToString(row->Values[COL_COURSE]) << setw(6)
       << valueToString(row->Values[COL_YEAR]) << setw(6)
       << valueToString(row->Values[COL_GPA]) << endl;
}

static void printHeader() {
  cout << left << setw(10) << "ID" << setw(12) << "FIRST" << setw(12) << "LAST"
       << setw(10) << "COURSE" << setw(6) << "YEAR" << setw(6) << "GPA" << endl;
  cout << string(56, '-') << endl;
}

static bool addStudent(int id, string first, string last, string course,
                       int year, double gpa) {
  Value row[6];
  row[COL_ID] = makeInt(id);
  row[COL_FIRST] = makeString(first);
  row[COL_LAST] = makeString(last);
  row[COL_COURSE] = makeString(course);
  row[COL_YEAR] = makeInt(year);
  row[COL_GPA] = makeDouble(gpa);
  return InsertRow(Students, row, Students->ColumnCount);
}

static void displayAll() {
  Row *rows[MaxRowReturn];
  int n = GetAllRows(Students, rows, MaxRowReturn);
  cout << "\nALL STUDENTS (" << CountRows(Students->RowHead) << ")\n";
  printHeader();
  for (int i = 0; i < n; i++) {
    printRow(rows[i]);
  }
}

static void searchStudent() {
  string column;
  string query;
  cout << "Search by column (ID/FIRST/LAST/COURSE/YEAR/GPA): ";
  cin >> column;
  cout << "Value: ";
  cin >> query;

  Value match;
  if (column == "ID" || column == "YEAR") {
    match = makeInt(stoi(query));
  } else if (column == "GPA") {
    match = makeDouble(stod(query));
  } else {
    match = makeString(query);
  }

  Row *found[MaxRowReturn];
  int linear = LinearSearch(Students, column, match, found, MaxRowReturn);
  cout << "\nLinear search: " << linear << " match(es)\n";
  printHeader();
  for (int i = 0; i < linear && i < MaxRowReturn; i++) {
    printRow(found[i]);
  }

  int binary = BinarySearch(Students, column, match, found, MaxRowReturn);
  cout << "Binary search: " << binary << " match(es)\n";
}

static void updateStudent() {
  int id;
  cout << "Student ID to update: ";
  cin >> id;

  Row *row = FindRow(Students, "ID", makeInt(id));
  if (row == nullptr) {
    cout << "Not found.\n";
    return;
  }

  string column;
  string value;
  cout << "Column to update: ";
  cin >> column;
  cout << "New value: ";
  cin >> value;

  Value newValue;
  if (column == "ID" || column == "YEAR") {
    newValue = makeInt(stoi(value));
  } else if (column == "GPA") {
    newValue = makeDouble(stod(value));
  } else {
    newValue = makeString(value);
  }

  if (UpdateRow(Students, row, column, newValue)) {
    cout << "Updated.\n";
  } else {
    cout << "Update failed (unknown column).\n";
  }
}

static void deleteStudent() {
  int id;
  cout << "Student ID to delete: ";
  cin >> id;

  if (DeleteRow(Students, "ID", makeInt(id))) {
    cout << "Deleted. It is now on top of the deleted stack.\n";
  } else {
    cout << "Not found.\n";
  }
}

static void sortStudents() {
  string column;
  int choice;
  cout << "Sort by (ID/FIRST/LAST/COURSE/YEAR/GPA): ";
  cin >> column;
  cout << "Algorithm (1 Bubble, 2 Selection, 3 Insertion): ";
  cin >> choice;
  int direction;
  cout << "Direction (1 Ascending, 2 Descending): ";
  cin >> direction;

  SortAlgorithm algo = (choice == 2)   ? SELECTION
                       : (choice == 3) ? INSERTION
                                       : BUBBLE;

  if (SortRows(Students, column, algo, direction == 1)) {
    cout << "Sorted.\n";
    displayAll();
  } else {
    cout << "Sort failed (unknown column or too many rows).\n";
  }
}

static void stackOperations() {
  Row *rows[MaxRowReturn];
  int n = DisplayStack(Students, rows, MaxRowReturn);

  cout << "\nRECENTLY DELETED (STACK)\n";
  if (n == 0) {
    cout << "Empty.\n";
    return;
  }

  cout << "Top\n ↓\n";
  for (int i = 0; i < n; i++) {
    cout << valueToString(rows[i]->Values[COL_ID]) << " "
         << valueToString(rows[i]->Values[COL_FIRST]) << " "
         << valueToString(rows[i]->Values[COL_LAST]) << endl;
  }

  int choice;
  cout << "\n1 Restore top, 2 Pop top, 0 Back: ";
  cin >> choice;

  if (choice == 1) {
    if (RestoreDeleted(Students)) {
      cout << "Restored.\n";
    } else {
      cout << "Stack empty.\n";
    }
  } else if (choice == 2) {
    Row *popped = PopDeleted(Students);
    if (popped != nullptr) {
      cout << "Popped " << valueToString(popped->Values[COL_LAST]) << endl;
      delete popped; // pop transfers ownership to us
    } else {
      cout << "Stack empty.\n";
    }
  }
}

static void queueOperations() {
  int choice;
  cout << "\nSERVICE QUEUE\n1 Enqueue a student, 2 Serve (dequeue), 3 Show "
          "queue, 0 Back: ";
  cin >> choice;

  if (choice == 1) {
    int id;
    cout << "Student ID: ";
    cin >> id;
    Row *row = FindRow(Students, "ID", makeInt(id));
    if (row != nullptr && Enqueue(Students, row)) {
      cout << "Queued.\n";
    } else {
      cout << "Not found.\n";
    }
  } else if (choice == 2) {
    Row *served = Dequeue(Students);
    if (served != nullptr) {
      cout << "Now serving: " << valueToString(served->Values[COL_FIRST]) << " "
           << valueToString(served->Values[COL_LAST]) << endl;
    } else {
      cout << "Queue empty.\n";
    }
  } else if (choice == 3) {
    Row *rows[MaxRowReturn];
    int n = DisplayQueue(Students, rows, MaxRowReturn);
    cout << "\nSTUDENT SERVICE QUEUE\n";
    for (int i = 0; i < n; i++) {
      cout << i + 1 << ". " << valueToString(rows[i]->Values[COL_FIRST]) << " "
           << valueToString(rows[i]->Values[COL_LAST]) << endl;
    }
    if (n == 0) {
      cout << "Empty.\n";
    }
  }
}

static void statistics() {
  cout << "\nSTUDENT STATISTICS\n";
  cout << "Total students: " << CountRows(Students->RowHead) << endl;

  ColumnStats gpa = ComputeStats(Students, "GPA");
  if (gpa.count > 0) {
    cout << "GPA average   : " << fixed << setprecision(2) << gpa.avg << endl;
    cout << "GPA highest   : " << gpa.max << endl;
    cout << "GPA lowest    : " << gpa.min << endl;
  }

  // Per-course counts, computed with a linear scan over the distinct values.
  string seen[MaxRowReturn];
  int seenCount = 0;
  Row *current = Students->RowHead;
  while (current != nullptr) {
    string course = valueToString(current->Values[COL_COURSE]);
    bool known = false;
    for (int i = 0; i < seenCount; i++) {
      if (seen[i] == course) {
        known = true;
        break;
      }
    }
    if (!known && seenCount < MaxRowReturn) {
      seen[seenCount] = course;
      seenCount += 1;
    }
    current = current->next;
  }

  cout << "\nStudents per course:\n";
  for (int i = 0; i < seenCount; i++) {
    int count = 0;
    Row *scan = Students->RowHead;
    while (scan != nullptr) {
      if (valueToString(scan->Values[COL_COURSE]) == seen[i]) {
        count += 1;
      }
      scan = scan->next;
    }
    cout << "  " << left << setw(8) << seen[i] << count << endl;
  }
}

static void seed() {
  addStudent(2026001, "Juan", "Dela Cruz", "BSCS", 1, 1.75);
  addStudent(2026003, "Pedro", "Cruz", "BSIT", 2, 2.00);
  addStudent(2026005, "Maria", "Santos", "BSCS", 2, 1.50);
  addStudent(2026007, "Ana", "Reyes", "ACT", 1, 1.25);
  addStudent(2026009, "Luis", "Bautista", "BSIT", 3, 1.90);
}

static void setup() {
  headlessDeleteDatabase("SCHOOL"); // clean slate on re-run
  headlessCreateNewDatabase("SCHOOL");
  DB = returnQueryADB("SCHOOL");

  CreateTable(DB, "STUDENTS");
  Students = SearchTable(DB, "STUDENTS");

  AddColumn(Students, "ID", COL_INT);
  AddColumn(Students, "FIRST", COL_STRING);
  AddColumn(Students, "LAST", COL_STRING);
  AddColumn(Students, "COURSE", COL_STRING);
  AddColumn(Students, "YEAR", COL_INT);
  AddColumn(Students, "GPA", COL_DOUBLE);

  COL_ID = FindColumn(Students, "ID");
  COL_FIRST = FindColumn(Students, "FIRST");
  COL_LAST = FindColumn(Students, "LAST");
  COL_COURSE = FindColumn(Students, "COURSE");
  COL_YEAR = FindColumn(Students, "YEAR");
  COL_GPA = FindColumn(Students, "GPA");
}

int main() {
  setup();
  seed();

  int choice = -1;
  while (choice != 10) {
    cout << "\n========================================\n";
    cout << " STUDENT RECORD MANAGEMENT SYSTEM\n";
    cout << "========================================\n";
    cout << "1. Add Student\n";
    cout << "2. Display All Students\n";
    cout << "3. Search Student\n";
    cout << "4. Update Student\n";
    cout << "5. Delete Student\n";
    cout << "6. Sort Students\n";
    cout << "7. Stack Operations\n";
    cout << "8. Queue Operations\n";
    cout << "9. Student Statistics\n";
    cout << "10. Exit\n";
    cout << "========================================\n";
    cout << "Enter your choice: ";

    if (!(cin >> choice)) {
      break;
    }

    switch (choice) {
    case 1: {
      int id, year;
      string first, last, course;
      double gpa;
      cout << "ID: ";
      cin >> id;
      cout << "First name: ";
      cin >> first;
      cout << "Last name: ";
      cin >> last;
      cout << "Course: ";
      cin >> course;
      cout << "Year: ";
      cin >> year;
      cout << "GPA: ";
      cin >> gpa;
      cout << (addStudent(id, first, last, course, year, gpa) ? "Added.\n"
                                                             : "Failed.\n");
      break;
    }
    case 2:
      displayAll();
      break;
    case 3:
      searchStudent();
      break;
    case 4:
      updateStudent();
      break;
    case 5:
      deleteStudent();
      break;
    case 6:
      sortStudents();
      break;
    case 7:
      stackOperations();
      break;
    case 8:
      queueOperations();
      break;
    case 9:
      statistics();
      break;
    case 10:
      cout << "Goodbye.\n";
      break;
    default:
      cout << "Invalid choice.\n";
      break;
    }
  }

  headlessDeleteDatabase("SCHOOL");
  return 0;
}
