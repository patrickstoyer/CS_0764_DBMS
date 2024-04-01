#include "Record.h"
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

Record::Record(char * key, char * data,
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
    free(key);
    free(data);
	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record * other)
{
    return (!(strncmp(this->key,other->key,constants::KEY_SIZE) > 0));
}

void Record::storeRecord (char * buffer, int * bufferIndexPtr, FILE * file, bool flushBuffer)
{
    if (*bufferIndexPtr + constants::KEY_SIZE + constants::RECORD_SIZE > constants::PAGE_SIZE)
    {
        // Save bufferIndex (= number of bytes stored to buffer so far) bytes
        fwrite(buffer, 1, *bufferIndexPtr, file);
        *bufferIndexPtr = 0;
    }
    // Copy KEY_SIZE bytes from key to buffer, and increment index
    strncpy(&buffer[*bufferIndexPtr],this->key,constants::KEY_SIZE);
    *bufferIndexPtr += constants::KEY_SIZE;
    // Same with rest of data
    strncpy(&buffer[*bufferIndexPtr],this->data,constants::RECORD_SIZE);
    *bufferIndexPtr += constants::RECORD_SIZE;
    // If flushBuffer is true (e.g. last record being scanned), flush buffer to file
    if (flushBuffer)
    {
        fwrite(buffer, 1, *bufferIndexPtr, file);
        *bufferIndexPtr = 0;
    }
	// 	b - Add to file
} // Record::storeRecord