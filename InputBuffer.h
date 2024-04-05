#include "Iterator.h"
#include "Record.h"
#include <stdio.h>

class InputBuffer
{
public:
    InputBuffer();
    InputBuffer(FILE * inputFile, char * buffer, char bufferType);
	~InputBuffer ();
	Record * get();
private:
    void read();
	FILE * _inputFile;
    char * buffer; 
    int bufferIndexPtr,
    char type ; // 1 = SSD / 2 = HDD
}; // class InputBuffer
