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

    fseek (_inputFile, 0, SEEK_END);   // non-portable
    _fileSize=ftell(_inputFile);
    fseek(_inputFile,0,0);
}
InputBuffer::~InputBuffer()
{
    delete [] _inputBuffer;
    fclose(_inputFile);
}
void InputBuffer::nullBuffer()
{
    _inputBuffer = nullptr;
    _inputFile = nullptr;
}
Record * InputBuffer::peek()
{
    char * data = new char [RECORD_SIZE];
    if ((fread(data,1,RECORD_SIZE,_inputFile) != 0) && (!feof(_inputFile)))
    {
        fseek(_inputFile,-RECORD_SIZE-1,SEEK_CUR);
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
        trackBuffer();

        return new Record(data,0);
    }
    char * lf = new char[1]{'~'};
    return new Record(lf,0);
}

void InputBuffer::ready(int skipIndex) {}
bool InputBuffer::storeNextAndSwap(Record& record, FILE * outputFile)
{
    return storeNextAndSwap(record,outputFile,false,-1);
}

bool InputBuffer::storeNextAndSwap(Record& record, FILE * outputFile, bool alwaysSwap,int lastCache)
{
    Record * nextRecord = next();
    nextRecord->storeRecord(outputFile,false);
    if (alwaysSwap)
    {
        record.~Record();
        record = *nextRecord;
        nextRecord->data = nullptr;
    }
    delete nextRecord;

    return false; // Always return false -- we cannot swap the input into the existing file
}

void InputBuffer::trackBuffer()
{
    // Buffering is handled automatically by C++ via setvbuf,
    //  so we are really just guessing at its behavior
    long long tell = ftell(_inputFile);
    if (_lastRead < tell)
    {
        long long bytesRead = (_lastRead + _pageSize > _fileSize) ? _fileSize - _lastRead : _pageSize;
        _lastRead += bytesRead;
        double latency = (_pageSize == HDD_PAGE_SIZE) ? 5 : 0.1;
        TOTAL_READ += bytesRead;
        TOTAL_LATENCY += latency;
        traceprintf("%s read of %lld bytes with latency %.2f ms (total I/O latency: %.2f)\n",(_pageSize == HDD_PAGE_SIZE) ? "HDD" : "SSD",bytesRead,latency,TOTAL_LATENCY);
    }
}
void InputBuffer::reset() {};
void InputBuffer::reset(int size, int dir, bool resetStreams, bool initializing){};

Record * InputBuffer::nextAndReplace()
{
    throw std::invalid_argument("Don't call nextAndReplace on InputBuffer");
}
