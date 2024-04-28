#include "InputBuffer.h"
#include "Record.h"
#include "PriorityQueue.h"
#include <cstdio>

InputBuffer::InputBuffer() = default;
InputBuffer::InputBuffer(const char * filename, char bufferType) : _lastRead(0)

{
    _inputFile = fopen(filename, "r");
    _pageSize = (bufferType == 1) ? HDD_PAGE_SIZE : SSD_PAGE_SIZE;
    _inputBuffer = new char[_pageSize];
    setvbuf(_inputFile,_inputBuffer,_IOFBF,_pageSize);

    _bytesReadCounter=0;

}
InputBuffer::~InputBuffer()
{
    trackBuffer(true);
    delete [] _inputBuffer;
    if (_inputFile!= nullptr) fclose(_inputFile);
}
void InputBuffer::nullBuffer()
{
    this->~InputBuffer();
    _inputBuffer = nullptr;
    _inputFile = nullptr;
}
Record * InputBuffer::peek()
{
    char * data = new char [RECORD_SIZE];
    if ((fread(data,1,RECORD_SIZE,_inputFile) != 0) && (!feof(_inputFile)))
    {
        int offset = (USE_NEWLINES) ? -RECORD_SIZE-1 : -RECORD_SIZE;
        fseek(_inputFile,offset,SEEK_CUR);
        return new Record(data,0);
    }
    delete [] data;
    char * lf = new char[1]{'~'};
    return new Record(lf,0);
}
Record * InputBuffer::peek(bool copy)
{
    return peek();
}
Record * InputBuffer::next()
{
    char * data = new char [RECORD_SIZE];

    if ((fread(data,1,RECORD_SIZE,_inputFile) != 0) && (!feof(_inputFile)))
    {
        _bytesReadCounter += RECORD_SIZE;
        trackBuffer(false);

        return new Record(data,0);
    }
    delete [] data;
    char * lf = new char[1]{'~'};
    return new Record(lf,0);
}

void InputBuffer::ready(int skipIndex) {}
bool InputBuffer::storeNextAndSwap(Record& record, FILE * outputFile)
{
    bool tmp = false;
    return storeNextAndSwap(record,outputFile,false,-1,tmp);
}

bool InputBuffer::storeNextAndSwap(Record& record, FILE * outputFile, bool alwaysSwap,int lastCache, bool& wasDuplicate)
{
    Record * nextRecord = next();
    if (!REMOVE_DUPES
        || (nextRecord->compare(PriorityQueue::_lastSaved)!=0)
        || !nextRecord->isDuplicate(PriorityQueue::_lastSaved))
    {
        nextRecord->storeRecord(outputFile, false);
    }
    else
    {
        wasDuplicate = true;
        // Update duplicate parity
        int recSize = (USE_NEWLINES) ? RECORD_SIZE - 1 : RECORD_SIZE;
        for (int i = 0; i < recSize; i++) {
            DUPLICATE_PARITY = (char) (DUPLICATE_PARITY ^ peek()->data[i]);
        }
    }
    PriorityQueue::_lastSaved.copy(*nextRecord);

    if (alwaysSwap)
    {
        record.~Record();
        record = *nextRecord;
        nextRecord->data = nullptr;
    }
    delete nextRecord;

    return false; // Always return false -- we cannot swap the input into the existing file
}

void InputBuffer::trackBuffer(bool flush)
{
    // Buffering is handled automatically by C++ via setvbuf,
    //  so we are really just guessing at its behavior
    if (!((_bytesReadCounter > _pageSize)||(flush && _bytesReadCounter >0))) return;
    long long bytesRead = (_bytesReadCounter > _pageSize) ? _pageSize : _bytesReadCounter;
    _bytesReadCounter -= bytesRead;
    double latency = (_pageSize == HDD_PAGE_SIZE) ? 5 : 0.1;
    TOTAL_READ += bytesRead;
    TOTAL_LATENCY += latency;
    //traceprintf("%s read of %lld bytes with latency %.1f ms (total I/O latency: %.1f)\n",(_pageSize == HDD_PAGE_SIZE) ? "HDD" : "SSD",bytesRead,latency,TOTAL_LATENCY);

}
void InputBuffer::reset() {};
void InputBuffer::reset(int size, int dir, bool resetStreams, bool initializing){};

Record * InputBuffer::nextAndReplace()
{
    throw std::invalid_argument("Don't call nextAndReplace on InputBuffer");
}
