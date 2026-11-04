// THIS IS THE TABLE FUNCTIONS, DO NOT TOUCH, THIS HAS NO CONFIGS, VISIT
// CONFIG.HPP FOR CONFIGURATION
#include "struct.hpp"
#include <iostream>
#include "config.hpp"
using namespace std;

Database* CreateDatabase(string name){ //Creating a new database for the engine
    Database* db = new Database;
    db->DatabaseName = name;

    return db;
}

bool CreateTable(Database* db, string tableName) {
    Node* Table = new Node;
    Table->DatabaseParent = db;
    Table->next = nullptr;
    Table->table = tableName;

    if (db->TableHead == nullptr) {
        db->TableHead = Table;
    }
    else {
        Node* Current = db->TableHead;

        while (Current->next != nullptr) {
            Current = Current->next;
        }

        Current->next = Table;
    }

    return true;
}

void DisplayTables(Database* db, bool EnablePFdbDefaults = true){
    if (db->TableHead == nullptr){
        if (EnablePFdbDefaults == true){
            cout << "ERROR INVALID OPERATIONS";
        }
    } else {
        Node* Current = db->TableHead;
        cout << "TABLES: " << endl;
        do{
            cout << Current->table;
            Current = Current->next;
        } while (Current != nullptr);
    }
}

Node* SearchTable(Database* db, string tableName){

    if (db == nullptr){
        return nullptr;
    }

    Node* Current = db->TableHead;
    do {
        if (Current == nullptr){
            return nullptr;
        }

        if (tableName == Current->table){
            break;
        };
        Current = Current->next;
    }while(Current != nullptr);


    return Current;
};

bool DeleteTable(Database* db, string tableName){
    
};