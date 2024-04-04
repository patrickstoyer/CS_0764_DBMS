#pragma once
#include <fstream>
#include <iostream>
#include <stdio.h>
#include "defs.h"

class Record
{
public:
	Record (char * data, int index);
   // Record (std::fstream& file); // Creates a record from next line of file
	virtual ~Record ();
	bool sortsBefore(Record * other); // Called to check sort order
    void storeRecord (char * buffer, int * bufferIndexPtr, FILE * file, bool flushBuffer);
    char * data;
private:
    int index; 
}; // class Record

extern int RECORD_SIZE;
extern int CACHE_SIZE;
extern int SSD_PAGE_SIZE;
extern int HDD_PAGE_SIZE;
extern int MEM_SIZE;
extern long long SSD_SIZE;
extern bool USE_NEWLINES;
extern unsigned SEED;