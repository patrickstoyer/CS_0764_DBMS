#include "Record.h"
#include "PriorityQueue.h"
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

Record::Record(char * data,
    int index) //: data(data), index(index)
{
//	TRACE (true);
    this->data = data;
    this->index = index;
}

Record::Record()
{
    this->data = nullptr;
}

Record::~Record ()
{
   delete [] data;
//	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record& other) const
{
    int cmp = strncmp(this->data,other.data,KEY_SIZE);
    bool retVal = (cmp <= 0); // Cmp>0 = sorts after cmp = 0 = match, cmp < 0 = sorts

    return retVal;
}

void Record::storeRecord (FILE * file, bool flush) const
{
    if (strncmp(this->data,&LATE_FENCE,1) == 0 ||strncmp(this->data,&EARLY_FENCE,1) ==0) return;
    // Note that buffering happens automatically, we add buffer array when we open the file
    fwrite(this->data,1,RECORD_SIZE,file);
    if (flush)
    {
        fflush(file);
    }
}

void Record::exchange(Record &other)
{
    char * dataTmp = other.data;
    int indexTmp = other.index;
    other.data = this->data;
    other.index = this->index;
    this->data = dataTmp;
    this->index = indexTmp;
}

Record::Record(Record &other)
{
    this->data = new char [RECORD_SIZE];
    strcpy(this->data,other.data);
    this->index = other.index;
}
// Record::storeRecord