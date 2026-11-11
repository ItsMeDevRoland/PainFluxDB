#pragma once
// WHEN ANNOUNCING OR MAKING UTILITY FUNCTION FOR PAINFLUXDB, PLEASE USE SNAKE
// CASE aka: count_Array() <-- has "_"

#include "struct.hpp"
#include <string>
using namespace std;

int count_Array(Database *Array, int ArrayMaximumSize);
string standardizedString(string Text);
void clearArrayContainer();

// Value helpers. The engine compares cells without knowing what they mean.
int compareValue(const Value &a, const Value &b);
bool valueEquals(const Value &a, const Value &b);
string valueToString(const Value &v);

Value makeInt(int v);
Value makeDouble(double v);
Value makeString(string v);
Value makeBool(bool v);

extern Containers ArrayContainer;
