#include "Record.h"
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

Record::Record(char * data,
    int index) //: data(data), index(index)
{ 
	TRACE (true);
    this->data = data;
    this->index = index;
}

Record::Record()
{
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
    free(data);
	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record * other)
{
    int cmp = strncmp(this->data,other->data,RECORD_SIZE);
    bool retVal = (!(cmp > 0));
    //std::cerr << "1COMP = " << cmp << " \n\t DATA = " << this->data << " \n\t OTHER = " << other.data << "\n";
    return retVal;
}

void Record::storeRecord (char * buffer, int * bufferIndexPtr, FILE * file, bool flushBuffer)
{
    // TODO Store to either SSD/HDD (pass file type as parameter)
    if (*bufferIndexPtr + RECORD_SIZE > HDD_PAGE_SIZE)
    {
        // Save bufferIndex (= number of bytes stored to buffer so far) bytes
        fwrite(buffer, 1, *bufferIndexPtr, file);
        *bufferIndexPtr = 0;
    }
    // Copy RECORD_SIZE bytes from record to buffer, and increment index
    strncpy(&buffer[*bufferIndexPtr],this->data,RECORD_SIZE);
    *bufferIndexPtr += RECORD_SIZE;
    // If flushBuffer is true (e.g. last record being scanned), flush buffer to file
    if (flushBuffer)
    {
        fwrite(buffer, 1, *bufferIndexPtr, file);
        *bufferIndexPtr = 0;
    }
} // Record::storeRecord