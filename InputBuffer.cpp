#include "InputBuffer.h"
#include "Record.h"
#include "PriorityQueue.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

InputBuffer::InputBuffer() 
{ 
}
InputBuffer::InputBuffer(const char * filename, char bufferType) : _bufferIndexPtr(0),_type(bufferType)

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

void InputBuffer::repair() {

}
