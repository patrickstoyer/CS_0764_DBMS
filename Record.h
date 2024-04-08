#pragma once
#include <iostream>
#include <stdio.h>
#include "defs.h"

class Record
{
public:
	Record (char * data, int index);
    Record (Record& other);
    Record (); 
	virtual ~Record ();
	bool sortsBefore(Record& other); // Called to check sort order
    void storeRecord (FILE * file, bool flush);
    void exchange(Record& other);
    char * data;
    int index;
private:
}; // class Record

extern int RECORD_SIZE;
extern int CACHE_SIZE;
extern int SSD_PAGE_SIZE;
extern int HDD_PAGE_SIZE;
extern int MEM_SIZE;
extern long long SSD_SIZE;
extern bool USE_NEWLINES;
extern int SEED;