// THIS IS WHERE YOU CONFIGURE EVERYTHING, this is THE MAIN CONTROL PANEL
#pragma once

const int TableColLimit =
    100; // Since we're Using Array instead of Vector, Limit is placed here,
         // modify depending on your USE CASE
const int TableRowLimit = TableColLimit; // Row limit per column, DO NOT MODIFY!
const int DatabaseLimit = 100; // Database Amount PainFluxDB can accomodate
const int TableLimit = 100;    // Table per database

const bool EnableDevTool = true; // enable the CLI built in within PainFluxDB

const int MaxAliasSize =
    10; // Max Alias Size, MUST NOT BE BELOW 5! feel free to indipendently
        // increase it above the 5 however!

const int MaxDBReturn = 10; // Max DB a Return can send

const int MaxArrayContainer = 100; //Tells how much stack of Container we can hold, By default the system will use 10
