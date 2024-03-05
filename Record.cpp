#include "Record.h"
#include <string.h>

Record::Record(unsigned char * key, unsigned char * pointer)
{
	TRACE (true);
    this->key = key;
    this->pointer = pointer;
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
