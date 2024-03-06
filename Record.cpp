#include "Record.h"
#include <string.h>
#include <fstream>

Record::Record(unsigned char * key, 
    unsigned char level, unsigned char file, unsigned int which)
{
	TRACE (true);
    this->key = key;
    this->level = level;
    this->file = file;
    this->which = which;
}

Record::Record(fstream File
    unsigned char level, unsigned char file, unsigned int which)
{
	TRACE (true);
    char * line = File.getline();
    this->key = line[constants::KEY_SIZE];
    this->level = level;
    this->file = file;
    this->which = which;
}

Record::~Record ()
{
	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record * other)
{
    return (memcmp(this->key, other->key, constants::KEY_SIZE) < 0);
}
