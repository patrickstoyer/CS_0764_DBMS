#include "Record.h"
#include <string.h>
#include <fstream>
#include <stdio.h>

Record::Record(std::vector<unsigned char> key, std::vector<unsigned char> data,
    int index)
{
	TRACE (true);
    this->key = key;
    this->data = data;
    this->index = index;
}
/*
Record::Record(std)
{
	TRACE (true);
  //  fprintf(stderr,"HELLOWFLLEIOW1");
    if (!file.is_open()) return;
    char line[constants::KEY_SIZE + constants::RECORD_SIZE];
    
    //fprintf(stderr,"HELLOWFLLEIOW2");
  //  file.getline(line,sizeof(line)/sizeof(*line));
   // fprintf(stderr,line);
    //fprintf(stderr,"POOPOO");
    while(file.peek()!=EOF)
    {
        file.getline(line,sizeof(line)/sizeof(*line));
        fprintf(stderr,line);
        fprintf(stderr,"POOPOO");
    }
    std::cout<<sizeof(line)/sizeof(*line);
    return;
}*/

Record::~Record ()
{
	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record * other)
{
    for (unsigned int i = 0; i < this->key.size(); i++)
    {
        if (this->key.at(i) < other->key.at(i))
        {
            return true;
        }
        else if (this->key.at(i) > other->key.at(i))
        {
            return false;
        }
    }
    return true;
   // return (memcmp(this->key, other->key, constants::KEY_SIZE) < 0);
}

void Record::storeRecord (std::vector<unsigned char> buffer, FILE* file, bool flushBuffer)
{
	//  a - If buffer full, save/clear
	// 	b - Add to file

    
    // 1. Check buffer size
    // 2. If full is true save to file, and clear
    // 3. Add to buffer
    // 4. If flushBuffer is true, save to file and clear
} // Record::storeRecord