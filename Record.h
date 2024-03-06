#pragma once

#include <fstream>
#include "defs.h"

class Record
{
public:
	Record (unsigned char * key, unsigned char * pointer);
    Record (fstream File); // Creates a record from next line of file
	virtual ~Record ();
	bool sortsBefore(Record * other); // Called to check sort order
    void storeRecord (fstream File);
private:
    unsigned char * key; // An array representing the key (variable size, passed as param)
    unsigned char * pointer; // A pointer to the record in memory (TODO: should this instead be three things level (e.g. SSD runs) + part (e.g. 3rd run) + offset (e.g. 100th entry))
}; // class Record

namespace constants
{
    int KEY_SIZE = 8; // Default to 8 bytes
	int RECORD_SIZE = 16; // Default to 16 bytes (note, this does not include the key)
	int PAGE_SIZE = 1000000; // Default to 1000000 bytes
} // namespace constants
