#include "InputBuffer.h"
#include "Record.h"
#include "PriorityQueue.h"
#include <cstdio>

InputBuffer::InputBuffer() = default;
InputBuffer::InputBuffer(const char * filename, char bufferType)

{
    _inputFile = fopen(filename, "r");
    int bufferSize = (bufferType == 1) ? HDD_PAGE_SIZE : SSD_PAGE_SIZE;
    _inputBuffer = new char[bufferSize];
    setvbuf(_inputFile,_inputBuffer,_IOFBF,bufferSize);
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
    if (fread(data,1,RECORD_SIZE,_inputFile) != 0)
    {
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

void InputBuffer::reset() {}
