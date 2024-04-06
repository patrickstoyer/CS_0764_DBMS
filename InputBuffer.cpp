#include "InputBuffer.h"
#include "Record.h"
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

InputBuffer::InputBuffer() 
{ 
}
InputBuffer::InputBuffer(char * filename, char bufferType) : _bufferIndexPtr(0),_type(bufferType)
{
    _inputFile = fopen(filename, "r");
	_inputBuffer = (char *)malloc((bufferType == 1) ? HDD_PAGE_SIZE : SSD_PAGE_SIZE);
}
InputBuffer::~InputBuffer()
{

}
Record * InputBuffer::get()
{
    char * data = (char *) malloc(RECORD_SIZE);
    int page_size = (_type == 1) ? HDD_PAGE_SIZE : SSD_PAGE_SIZE;

    int charsToRead = (_bufferIndexPtr + RECORD_SIZE > page_size) ? (page_size - _bufferIndexPtr) : RECORD_SIZE;
    // Copy RECORD_SIZE bytes from record to buffer, and increment index
    strncpy(&_inputBuffer[_bufferIndexPtr],data,charsToRead);
    if (charsToRead < RECORD_SIZE)
    {
        charsToRead = RECORD_SIZE - charsToRead;
        read();
        strncpy(&_inputBuffer[_bufferIndexPtr],data,charsToRead);
    }
    return new Record(data,0);
}
void InputBuffer::read()
{
    int page_size = (_type == 1) ? HDD_PAGE_SIZE : SSD_PAGE_SIZE;
    fgets(_inputBuffer, page_size + 1, _inputFile); // + 1 because num - 1 chars are read
    _bufferIndexPtr = 0;
}