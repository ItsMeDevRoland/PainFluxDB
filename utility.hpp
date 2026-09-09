#pragma once
// WHEN ANNOUNCING OR MAKING UTILITY FUNCTION FOR PAINFLUXDB, PLEASE USE SNAKE
// CASE aka: count_Array() <-- has "_"

#include "struct.hpp"
#include <string>
using namespace std;

int count_Array(Database *Array, int ArrayMaximumSize);
string standardizedString(string Text);

extern Containers ArrayContainer;
