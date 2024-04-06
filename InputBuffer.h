#include "Iterator.h"
#include "Record.h"
#include <stdio.h>

class InputBuffer
{
public:
    InputBuffer();
    InputBuffer(char * inputFile, char bufferType);
	~InputBuffer ();
	Record * get();
private:
    void read();
	FILE * _inputFile;
    char * _inputBuffer; 
    int _bufferIndexPtr;
    char _type ; // 1 = HDD / 2 = SSD
}; // class InputBuffer
