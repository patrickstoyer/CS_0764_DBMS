#pragma once
#include <iostream>
#include <cstdio>
#include "defs.h"

class Record
{
public:
	Record (char * data, int index);
    Record (Record& other);
    Record (); 
	virtual ~Record ();
	bool sortsBefore(Record& other) const; // Called to check sort order
    void storeRecord (FILE * file, bool flush) const;
    void exchange(Record& other);
    void copy(Record& other);
    char * data;
    int index{};

    int compare(Record &other) const;

    bool isDuplicate(Record &other) const;

    static void trackBuffer(bool flush);
}; // class Record

extern int RECORD_SIZE;
extern int KEY_SIZE;
extern int CACHE_SIZE;
extern int SSD_PAGE_SIZE;
extern int HDD_PAGE_SIZE;
[[maybe_unused]] extern int MEM_SIZE;
extern long long SSD_SIZE;
extern bool USE_NEWLINES;
extern bool REMOVE_DUPES;
extern int SEED;
extern double TOTAL_LATENCY;
extern long long BYTES_WRITTEN_SSD;
extern long long BYTES_WRITTEN_HDD;
extern long long BYTES_WRITTEN_COUNTER;
extern bool ALWAYS_HDD;
extern char DUPLICATE_PARITY;