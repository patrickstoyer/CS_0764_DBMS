#include "Record.h"
#include "PriorityQueue.h"
#include <cstring>
#include <cstdio>

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
    return (compare(other)<=0);
}
int Record::compare(Record& other) const
{
    int keySize = KEY_SIZE;
    if ((this->data[0] == LATE_FENCE) || (this->data[0] == EARLY_FENCE) || (other.data[0] == LATE_FENCE) || (other.data[0] == EARLY_FENCE)) 
    {
        keySize = 1;
    }
    int cmp = strncmp(this->data,other.data,keySize);
    return cmp;
}
bool Record::isDuplicate(Record& other) const
{
    if ((this->data[0] == LATE_FENCE) || (this->data[0] == EARLY_FENCE) || (other.data[0] == LATE_FENCE) || (other.data[0] == EARLY_FENCE)) return false;
    int cmp = strncmp(this->data, other.data, RECORD_SIZE);
    bool retVal = (cmp == 0); // Cmp>0 = sorts after cmp = 0 = match, cmp < 0 = sorts
    return retVal;
}

void Record::storeRecord (FILE * file, bool flush) const
 {
    if ((this->data[0] == LATE_FENCE) ||(this->data[0] == EARLY_FENCE)) return;
    // Note that buffering happens automatically, we add buffer array when we open the file
    fwrite(this->data,1,RECORD_SIZE,file);
    if (flush)
    {
        fflush(file);
    }
    trackBuffer(flush);
}
void Record::trackBuffer (bool flush)
{

    bool isSsd = (!ALWAYS_HDD)&&(BYTES_WRITTEN_SSD + RECORD_SIZE < SSD_SIZE);
    if (isSsd)
    {
        BYTES_WRITTEN_SSD += RECORD_SIZE;
    } else
    {
        BYTES_WRITTEN_HDD += RECORD_SIZE;
    }
    TOTAL_WRITTEN += RECORD_SIZE;
    BYTES_WRITTEN_COUNTER += RECORD_SIZE;
    long long pageSize = (isSsd)? SSD_PAGE_SIZE : HDD_PAGE_SIZE;
    if (flush || (BYTES_WRITTEN_COUNTER >= pageSize))
    {
        double latency = (!isSsd) ? 5 : 0.1;
        TOTAL_LATENCY += latency;
        traceprintf("%s write of %lld bytes with latency %.2f ms (total I/O latency: %.2f)\n",(!isSsd) ? "HDD" : "SSD",pageSize,latency,TOTAL_LATENCY);
        BYTES_WRITTEN_COUNTER = BYTES_WRITTEN_COUNTER % pageSize;
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
void Record::copy(Record &other)
{
    int dataSize = ((other.data[0] == LATE_FENCE) ||(other.data[0] == EARLY_FENCE)) ? 1 : RECORD_SIZE;
    delete [] this->data;
    this->data = new char[dataSize];
    strncpy(this->data,other.data,dataSize);
}
Record::Record(Record &other)
{
    int recSize = ((other.data[0] == LATE_FENCE) ||(other.data[0] == EARLY_FENCE)) ? 1 : RECORD_SIZE;
    this->data = new char [recSize];
    strncpy(this->data,other.data,recSize);
    this->index = other.index;
}
// Record::storeRecord