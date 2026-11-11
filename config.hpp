// THIS IS WHERE YOU CONFIGURE EVERYTHING, this is THE MAIN CONTROL PANEL
#pragma once

// Every limit below is a hard cap. The engine never grows past these, so they
// are the first thing to change when the application needs more headroom.
const int TableColLimit = 100; // columns per table
const int DatabaseLimit = 100; // databases PainFluxDB can hold
const int TableLimit = 100;    // tables per database

const bool EnableDevTool = true; // enables the built-in dev CLI

const int MaxAliasSize = 10;       // aliases per command, MUST NOT BE BELOW 5
const int MaxDBReturn = 10;        // databases a query can return
const int MaxTableSearch = 100;    // tables a search can return
const int MaxArrayContainer = 100; // slots in the shared Containers buckets
const int MaxRowReturn = 1024;     // rows a search/sort snapshot can hold
