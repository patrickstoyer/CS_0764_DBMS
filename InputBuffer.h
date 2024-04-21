#include "Iterator.h"
#include "Record.h"
#include "InputStream.h"
#include <stdio.h>

class InputBuffer : public InputStream
{
public:
    InputBuffer();
    InputBuffer(const char * inputFile, char bufferType);
	~InputBuffer ();
    Record * next() override;
    Record * peek() override;
    void ready(int skipIndex) override;
    bool storeNextAndSwap(Record& record, FILE * outputFile) override;
private:
	FILE * _inputFile{};
    char * _inputBuffer{};
    int _bufferIndexPtr{};
    char _type{} ; // 1 = HDD / 2 = SSD
}; // class InputBuffer
