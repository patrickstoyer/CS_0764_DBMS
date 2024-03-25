#pragma once
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "defs.h"
#include "Iterator.h"

class Record
{
public:
	Record (std::vector<unsigned char> key, std::vector<unsigned char> data, int index);
   // Record (std::fstream& file); // Creates a record from next line of file
	virtual ~Record ();
	bool sortsBefore(Record * other); // Called to check sort order
    void storeRecord (std::vector<unsigned char> buffer, FILE* file, bool flushBuffer);
    std::vector<unsigned char> key; // An array representing the key (variable size, passed as param)
    std::vector<unsigned char> data;
private:
    int index; // The string of data (other than key)
}; // class Record

namespace constants
{
    static int KEY_SIZE = 8; // Default to 8 bytes
	static int RECORD_SIZE = 16; // Default to 16 bytes (note, this does not include the key)
    static int CACHE_SIZE = 1000;
	static int PAGE_SIZE = 10000; // Default to 10000 bytes
    static int MEM_SIZE = 10000000; // Default to 1000000 bytes
    static int SSD_SIZE = 100000000; // Default to 1000000 bytes
} // namespace constants