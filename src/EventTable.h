/**
 * @file EventTable.h
 * @date 24.03.2020
 * @author Grandeur Technologies
 *
 * Copyright (c) 2019 Grandeur Technologies LLP. All rights reserved.
 * This file is part of the Arduino SDK for Grandeur Cloud.
 *
 */

#include <cstdlib>
#include <string>
#include "apollotypes.h"

#ifndef EVENTTABLE_H_
#define EVENTTABLE_H_

/*TYPEDEFS*/
// EventID
typedef ApolloID EventID;
//EventData
typedef Callback EventData;
// Event table data
typedef struct {
  EventID id;
  EventData data;
} EventTableData;
// Event table key
typedef std::string EventTableKey;

// Event table size
const int T_S = 16;

// Routes Array
extern const char* routes[T_S];

// Event table entry
class EventTableEntry {
  public:
    EventTableKey k;
    EventTableData v;
    EventTableEntry* next;
    EventTableEntry(EventTableKey k, EventTableData v);
};

class EventTable {
  private:
    EventTableEntry **t;
  public:
    // Constructor for a hashtable
    EventTable();
    // Hash function for Event Table
    int hashFunc(EventTableKey k);
    // Method to insert an entry into the hashtable
    int insert(EventTableKey k, EventID id, EventData data);
    // Method to remove an entry from the hashtable
    int remove(EventTableKey k, EventID id);
    // Method to find and remove an entry from the hashtable
    EventData findAndRemove(EventTableKey k, EventID id);
    void print();
    // Destructor for a hashtable
    ~EventTable();
};

#endif